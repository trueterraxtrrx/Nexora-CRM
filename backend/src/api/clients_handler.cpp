#include "api/clients_handler.hpp"
#include "core/database.hpp"
#include "core/middleware.hpp"
#include "utils/response.hpp"
#include "utils/audit.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <algorithm>
#include <vector>

namespace crm::api {

using namespace crm::core;
using namespace crm::utils;
using json = nlohmann::json;

// ─── Helper: row → json ──────────────────────────────────────────────────────
static json client_to_json(const pqxx::row& r) {
    return {
        {"id",           r["id"].as<int>()},
        {"company_id",   r["company_id"].as<int>()},
        {"name",         r["name"].as<std::string>()},
        {"email",        r["email"].is_null()        ? nullptr : json(r["email"].as<std::string>())},
        {"phone",        r["phone"].is_null()        ? nullptr : json(r["phone"].as<std::string>())},
        {"company_name", r["company_name"].is_null() ? nullptr : json(r["company_name"].as<std::string>())},
        {"notes",        r["notes"].is_null()        ? nullptr : json(r["notes"].as<std::string>())},
        {"tags",         r["tags"].is_null()         ? nullptr : json(r["tags"].as<std::string>())},
        {"is_active",    r["is_active"].as<bool>()},
        {"created_at",   r["created_at"].as<std::string>()},
        {"updated_at",   r["updated_at"].as<std::string>()},
    };
}

void register_clients_routes(AppType& app) {

    // ── GET /api/v1/clients  ───────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/clients").methods(crow::HTTPMethod::Get)
    ([&app](const crow::request& req, crow::response& res) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        auto search    = req.url_params.get("search");
        auto is_active = req.url_params.get("is_active");
        auto tag       = req.url_params.get("tag");
        int skip  = req.url_params.get("skip")  ? std::stoi(req.url_params.get("skip"))  : 0;
        int limit = req.url_params.get("limit") ? std::stoi(req.url_params.get("limit")) : 50;
        limit = std::min(limit, 200);

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                // Динамически строим WHERE
                std::string where = "WHERE company_id=" + txn.quote(claims.company_id);

                if (is_active) {
                    where += " AND is_active=" + txn.quote(std::string(is_active) == "true");
                }
                if (search) {
                    std::string like = "%" + std::string(search) + "%";
                    where += fmt::format(
                        " AND (name ILIKE {0} OR email ILIKE {0} OR phone ILIKE {0} OR company_name ILIKE {0})",
                        txn.quote(like)
                    );
                }
                if (tag) {
                    where += " AND tags ILIKE " + txn.quote("%" + std::string(tag) + "%");
                }

                where += fmt::format(" ORDER BY created_at DESC LIMIT {} OFFSET {}", limit, skip);

                std::string sql = "SELECT id,company_id,name,email,phone,company_name,"
                                  "notes,tags,is_active,created_at,updated_at "
                                  "FROM clients " + where;

                auto rows = txn.exec(sql);

                json result = json::array();
                for (const auto& row : rows)
                    result.push_back(client_to_json(row));

                return json_ok(result);
            });
        } catch (const std::exception& e) {
            spdlog::error("List clients error: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── POST /api/v1/clients ───────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/clients").methods(crow::HTTPMethod::Post)
    ([&app](const crow::request& req, crow::response& res) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        auto body = parse_body(req);
        if (!body || !body->contains("name")) {
            res = json_error(400, "Поле 'name' обязательно");
            return;
        }

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                // Проверяем лимит плана
                auto limit_row = txn.exec_params1(
                    "SELECT max_clients FROM companies WHERE id=$1", claims.company_id
                );
                int max_clients = limit_row["max_clients"].as<int>();

                auto count_row = txn.exec_params1(
                    "SELECT COUNT(*) FROM clients WHERE company_id=$1 AND is_active=TRUE",
                    claims.company_id
                );
                if (count_row[0].as<int>() >= max_clients)
                    return json_error(403,
                        fmt::format("Достигнут лимит клиентов ({}). Обновите подписку.", max_clients));

                auto name         = (*body)["name"].get<std::string>();
                auto email        = get_optional<std::string>(*body, "email");
                auto phone        = get_optional<std::string>(*body, "phone");
                auto company_name = get_optional<std::string>(*body, "company_name");
                auto notes        = get_optional<std::string>(*body, "notes");
                auto tags         = get_optional<std::string>(*body, "tags");

                auto row = txn.exec_params1(
                    "INSERT INTO clients(company_id,name,email,phone,company_name,notes,tags) "
                    "VALUES($1,$2,NULLIF($3,''),NULLIF($4,''),NULLIF($5,''),NULLIF($6,''),NULLIF($7,'')) "
                    "RETURNING id,company_id,name,email,phone,company_name,notes,tags,is_active,created_at,updated_at",
                    claims.company_id, name,
                    email.value_or(""),
                    phone.value_or(""),
                    company_name.value_or(""),
                    notes.value_or(""),
                    tags.value_or("")
                );

                int new_id = row["id"].as<int>();
                write_audit(txn, claims.company_id, claims.user_id,
                    "create_client", "client", new_id, {{"name", name}});

                return json_created(client_to_json(row));
            });
        } catch (const std::exception& e) {
            spdlog::error("Create client error: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── GET /api/v1/clients/:id ────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/clients/<int>").methods(crow::HTTPMethod::Get)
    ([&app](const crow::request& req, crow::response& res, int client_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT id,company_id,name,email,phone,company_name,notes,tags,is_active,created_at,updated_at "
                    "FROM clients WHERE id=$1 AND company_id=$2",
                    client_id, claims.company_id
                );
                if (rows.empty()) return json_error(404, "Клиент не найден");
                return json_ok(client_to_json(rows[0]));
            });
        } catch (const std::exception& e) {
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── PATCH /api/v1/clients/:id ──────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/clients/<int>").methods(crow::HTTPMethod::Patch)
    ([&app](const crow::request& req, crow::response& res, int client_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        auto body = parse_body(req);
        if (!body) { res = json_error(400, "Invalid JSON"); return; }

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT id FROM clients WHERE id=$1 AND company_id=$2",
                    client_id, claims.company_id
                );
                if (rows.empty()) return json_error(404, "Клиент не найден");

                // Строим SET динамически
                std::vector<std::pair<std::string,std::string>> updates;
                if (body->contains("name"))         updates.push_back({"name",         (*body)["name"].get<std::string>()});
                if (body->contains("email"))        updates.push_back({"email",        (*body)["email"].is_null() ? "" : (*body)["email"].get<std::string>()});
                if (body->contains("phone"))        updates.push_back({"phone",        (*body)["phone"].is_null() ? "" : (*body)["phone"].get<std::string>()});
                if (body->contains("company_name")) updates.push_back({"company_name", (*body)["company_name"].is_null() ? "" : (*body)["company_name"].get<std::string>()});
                if (body->contains("notes"))        updates.push_back({"notes",        (*body)["notes"].is_null() ? "" : (*body)["notes"].get<std::string>()});
                if (body->contains("is_active"))    updates.push_back({"is_active",    (*body)["is_active"].get<bool>() ? "true" : "false"});

                if (updates.empty()) return json_error(400, "Нет полей для обновления");

                std::string set_clause = "updated_at=NOW()";
                for (auto& [col, val] : updates) {
                    set_clause += fmt::format(",{}={}", col, txn.quote(val));
                }

                std::string sql = "UPDATE clients SET " + set_clause +
                    fmt::format(" WHERE id={} AND company_id={}"
                    " RETURNING id,company_id,name,email,phone,company_name,notes,tags,is_active,created_at,updated_at",
                    txn.quote(client_id), txn.quote(claims.company_id));

                auto updated = txn.exec1(sql);
                write_audit(txn, claims.company_id, claims.user_id,
                    "update_client", "client", client_id, *body);

                return json_ok(client_to_json(updated));
            });
        } catch (const std::exception& e) {
            spdlog::error("Update client error: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── DELETE /api/v1/clients/:id ─────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/clients/<int>").methods(crow::HTTPMethod::Delete)
    ([&app](const crow::request& req, crow::response& res, int client_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_role(req, res, ctx, {"admin", "manager"})) return;
        auto& claims = *ctx.claims;

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT name FROM clients WHERE id=$1 AND company_id=$2",
                    client_id, claims.company_id
                );
                if (rows.empty()) return json_error(404, "Клиент не найден");
                auto name = rows[0]["name"].as<std::string>();

                txn.exec_params("DELETE FROM clients WHERE id=$1", client_id);
                write_audit(txn, claims.company_id, claims.user_id,
                    "delete_client", "client", client_id, {{"name", name}});

                return json_no_content();
            });
        } catch (const std::exception& e) {
            spdlog::error("Delete client error: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── GET /api/v1/clients/:id/interactions ──────────────────────────────
    CROW_ROUTE(app, "/api/v1/clients/<int>/interactions").methods(crow::HTTPMethod::Get)
    ([&app](const crow::request& req, crow::response& res, int client_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                // Проверяем владение клиентом
                auto check = txn.exec_params(
                    "SELECT id FROM clients WHERE id=$1 AND company_id=$2",
                    client_id, claims.company_id
                );
                if (check.empty()) return json_error(404, "Клиент не найден");

                auto rows = txn.exec_params(
                    "SELECT id,client_id,user_id,type,content,created_at "
                    "FROM client_interactions WHERE client_id=$1 ORDER BY created_at DESC",
                    client_id
                );

                json result = json::array();
                for (const auto& r : rows) {
                    result.push_back({
                        {"id",         r["id"].as<int>()},
                        {"client_id",  r["client_id"].as<int>()},
                        {"user_id",    r["user_id"].as<int>()},
                        {"type",       r["type"].as<std::string>()},
                        {"content",    r["content"].as<std::string>()},
                        {"created_at", r["created_at"].as<std::string>()},
                    });
                }
                return json_ok(result);
            });
        } catch (const std::exception& e) {
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── POST /api/v1/clients/:id/interactions ─────────────────────────────
    CROW_ROUTE(app, "/api/v1/clients/<int>/interactions").methods(crow::HTTPMethod::Post)
    ([&app](const crow::request& req, crow::response& res, int client_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        auto body = parse_body(req);
        if (!body || !body->contains("type") || !body->contains("content")) {
            res = json_error(400, "type и content обязательны");
            return;
        }

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto check = txn.exec_params(
                    "SELECT id FROM clients WHERE id=$1 AND company_id=$2",
                    client_id, claims.company_id
                );
                if (check.empty()) return json_error(404, "Клиент не найден");

                auto type    = (*body)["type"].get<std::string>();
                auto content = (*body)["content"].get<std::string>();

                auto row = txn.exec_params1(
                    "INSERT INTO client_interactions(client_id,user_id,type,content) "
                    "VALUES($1,$2,$3,$4) RETURNING id,client_id,user_id,type,content,created_at",
                    client_id, claims.user_id, type, content
                );

                return json_created({
                    {"id",         row["id"].as<int>()},
                    {"client_id",  row["client_id"].as<int>()},
                    {"user_id",    row["user_id"].as<int>()},
                    {"type",       row["type"].as<std::string>()},
                    {"content",    row["content"].as<std::string>()},
                    {"created_at", row["created_at"].as<std::string>()},
                });
            });
        } catch (const std::exception& e) {
            spdlog::error("Add interaction error: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });
}

} // namespace crm::api
