#include "api/finance_handler.hpp"
#include "core/database.hpp"
#include "core/middleware.hpp"
#include "utils/response.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <ctime>

namespace crm::api {

using namespace crm::core;
using namespace crm::utils;
using json = nlohmann::json;

static json finance_to_json(const pqxx::row& r) {
    return {
        {"id",          r["id"].as<int>()},
        {"company_id",  r["company_id"].as<int>()},
        {"type",        r["type"].as<std::string>()},
        {"amount",      r["amount"].as<double>()},
        {"currency",    r["currency"].as<std::string>()},
        {"description", r["description"].is_null() ? nullptr : json(r["description"].as<std::string>())},
        {"category",    r["category"].is_null()    ? nullptr : json(r["category"].as<std::string>())},
        {"date",        r["date"].as<std::string>()},
        {"client_id",   r["client_id"].is_null()   ? nullptr : json(r["client_id"].as<int>())},
        {"created_at",  r["created_at"].as<std::string>()},
    };
}

void register_finance_routes(AppType& app) {

    // ── GET /api/v1/finance ───────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/finance").methods(crow::HTTPMethod::Get)
    ([](const crow::request& req, crow::response& res, AppType::context& ctx) {
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.get<AuthMiddleware>().claims;

        int skip  = req.url_params.get("skip")  ? std::stoi(req.url_params.get("skip"))  : 0;
        int limit = req.url_params.get("limit") ? std::stoi(req.url_params.get("limit")) : 100;
        limit = std::min(limit, 500);

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                std::string where = "WHERE company_id=$1";
                pqxx::params params;
                params.append(claims.company_id);
                int idx = 2;

                if (auto t = req.url_params.get("type")) {
                    where += fmt::format(" AND type=${}::finance_type", idx++);
                    params.append(std::string(t));
                }
                if (auto cat = req.url_params.get("category")) {
                    where += fmt::format(" AND category=${}", idx++);
                    params.append(std::string(cat));
                }
                if (auto df = req.url_params.get("date_from")) {
                    where += fmt::format(" AND date>=${}", idx++);
                    params.append(std::string(df));
                }
                if (auto dt = req.url_params.get("date_to")) {
                    where += fmt::format(" AND date<=${}", idx++);
                    params.append(std::string(dt));
                }

                where += fmt::format(" ORDER BY date DESC LIMIT ${} OFFSET ${}", idx, idx+1);
                params.append(limit);
                params.append(skip);

                auto rows = txn.exec(
                    "SELECT id,company_id,type,amount,currency,description,category,"
                    "date,client_id,created_at FROM finances " + where,
                    params
                );

                json result = json::array();
                for (const auto& row : rows) result.push_back(finance_to_json(row));
                return json_ok(result);
            });
        } catch (const std::exception& e) {
            spdlog::error("List finance: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── POST /api/v1/finance ──────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/finance").methods(crow::HTTPMethod::Post)
    ([](const crow::request& req, crow::response& res, AppType::context& ctx) {
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.get<AuthMiddleware>().claims;

        auto body = parse_body(req);
        if (!body || !body->contains("type") || !body->contains("amount")) {
            res = json_error(400, "type и amount обязательны");
            return;
        }

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto type        = (*body)["type"].get<std::string>();
                auto amount      = (*body)["amount"].get<double>();
                auto currency    = body->contains("currency")    ? (*body)["currency"].get<std::string>()    : "RUB";
                auto description = get_optional<std::string>(*body, "description");
                auto category    = get_optional<std::string>(*body, "category");
                auto date_str    = get_optional<std::string>(*body, "date");

                auto row = txn.exec_params1(
                    "INSERT INTO finances(company_id,type,amount,currency,description,category,date) "
                    "VALUES($1,$2::finance_type,$3,$4,NULLIF($5,''),NULLIF($6,''),"
                    "COALESCE(NULLIF($7,'')::timestamptz, NOW())) "
                    "RETURNING id,company_id,type,amount,currency,description,category,date,client_id,created_at",
                    claims.company_id, type, amount, currency,
                    description.value_or(""), category.value_or(""), date_str.value_or("")
                );

                return json_created(finance_to_json(row));
            });
        } catch (const std::exception& e) {
            spdlog::error("Create finance: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── DELETE /api/v1/finance/:id ────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/finance/<int>").methods(crow::HTTPMethod::Delete)
    ([](const crow::request& req, crow::response& res, AppType::context& ctx, int rec_id) {
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.get<AuthMiddleware>().claims;
        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT id FROM finances WHERE id=$1 AND company_id=$2",
                    rec_id, claims.company_id
                );
                if (rows.empty()) return json_error(404, "Запись не найдена");
                txn.exec_params("DELETE FROM finances WHERE id=$1", rec_id);
                return json_no_content();
            });
        } catch (const std::exception& e) {
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── GET /api/v1/finance/report ────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/finance/report").methods(crow::HTTPMethod::Get)
    ([](const crow::request& req, crow::response& res, AppType::context& ctx) {
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.get<AuthMiddleware>().claims;

        // Год по умолчанию — текущий
        time_t now = time(nullptr);
        tm* tm_now = localtime(&now);
        int year = req.url_params.get("year") ?
            std::stoi(req.url_params.get("year")) : (tm_now->tm_year + 1900);

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                // Итоги
                auto totals = txn.exec_params(
                    "SELECT type, SUM(amount) as total "
                    "FROM finances WHERE company_id=$1 AND EXTRACT(year FROM date)=$2 "
                    "GROUP BY type",
                    claims.company_id, year
                );

                double total_income = 0, total_expense = 0;
                for (const auto& r : totals) {
                    if (r["type"].as<std::string>() == "income")
                        total_income = r["total"].as<double>();
                    else
                        total_expense = r["total"].as<double>();
                }

                // По категориям
                auto by_cat = txn.exec_params(
                    "SELECT COALESCE(category,'Без категории') as cat, type, SUM(amount) as total "
                    "FROM finances WHERE company_id=$1 AND EXTRACT(year FROM date)=$2 "
                    "GROUP BY cat, type ORDER BY cat",
                    claims.company_id, year
                );

                json categories = json::object();
                for (const auto& r : by_cat) {
                    auto cat  = r["cat"].as<std::string>();
                    auto type = r["type"].as<std::string>();
                    auto amt  = r["total"].as<double>();
                    if (!categories.contains(cat))
                        categories[cat] = {{"income", 0.0}, {"expense", 0.0}};
                    categories[cat][type] = amt;
                }

                // По месяцам
                auto by_month_rows = txn.exec_params(
                    "SELECT EXTRACT(month FROM date) as month, type, SUM(amount) as total "
                    "FROM finances WHERE company_id=$1 AND EXTRACT(year FROM date)=$2 "
                    "GROUP BY month, type ORDER BY month",
                    claims.company_id, year
                );

                // Инициализируем все 12 месяцев нулями
                std::array<std::pair<double,double>, 12> months_data{};
                for (const auto& r : by_month_rows) {
                    int m = static_cast<int>(r["month"].as<double>()) - 1;
                    if (r["type"].as<std::string>() == "income")
                        months_data[m].first  = r["total"].as<double>();
                    else
                        months_data[m].second = r["total"].as<double>();
                }

                json by_month = json::array();
                for (int i = 0; i < 12; ++i) {
                    by_month.push_back({
                        {"month",   i + 1},
                        {"income",  months_data[i].first},
                        {"expense", months_data[i].second},
                    });
                }

                return json_ok({
                    {"total_income",  total_income},
                    {"total_expense", total_expense},
                    {"profit",        total_income - total_expense},
                    {"by_category",   categories},
                    {"by_month",      by_month},
                });
            });
        } catch (const std::exception& e) {
            spdlog::error("Finance report: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });
}

} // namespace crm::api
