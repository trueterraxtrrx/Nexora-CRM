#include "api/users_handler.hpp"
#include "core/database.hpp"
#include "core/middleware.hpp"
#include "core/password.hpp"
#include "utils/response.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <vector>

namespace crm::api {

using namespace crm::core;
using namespace crm::utils;
using json = nlohmann::json;

static json user_to_json(const pqxx::row& r) {
    return {
        {"id",               r["id"].as<int>()},
        {"company_id",       r["company_id"].as<int>()},
        {"email",            r["email"].as<std::string>()},
        {"full_name",        r["full_name"].is_null() ? nullptr : json(r["full_name"].as<std::string>())},
        {"role",             r["role"].as<std::string>()},
        {"is_active",        r["is_active"].as<bool>()},
        {"notify_email",     r["notify_email"].as<bool>()},
        {"telegram_chat_id", r["telegram_chat_id"].is_null() ? nullptr : json(r["telegram_chat_id"].as<std::string>())},
        {"created_at",       r["created_at"].as<std::string>()},
        {"last_login",       r["last_login"].is_null() ? nullptr : json(r["last_login"].as<std::string>())},
    };
}

void register_users_routes(AppType& app) {

    // ── GET /api/v1/users ─────────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/users").methods(crow::HTTPMethod::Get)
    ([&app](const crow::request& req, crow::response& res) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT id,company_id,email,full_name,role,is_active,"
                    "notify_email,telegram_chat_id,created_at,last_login "
                    "FROM users WHERE company_id=$1 ORDER BY created_at ASC",
                    claims.company_id
                );
                json result = json::array();
                for (const auto& row : rows) result.push_back(user_to_json(row));
                return json_ok(result);
            });
        } catch (const std::exception& e) {
            spdlog::error("List users: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── POST /api/v1/users  (только admin) ───────────────────────────────
    CROW_ROUTE(app, "/api/v1/users").methods(crow::HTTPMethod::Post)
    ([&app](const crow::request& req, crow::response& res) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_role(req, res, ctx, {"admin"})) return;
        auto& claims = *ctx.claims;

        auto body = parse_body(req);
        if (!body || !body->contains("email") || !body->contains("password")) {
            res = json_error(400, "email и password обязательны");
            return;
        }

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                // Лимит пользователей по плану
                auto lim = txn.exec_params1(
                    "SELECT max_users FROM companies WHERE id=$1", claims.company_id
                );
                int max_users = lim["max_users"].as<int>();

                auto cnt = txn.exec_params1(
                    "SELECT COUNT(*) FROM users WHERE company_id=$1 AND is_active=TRUE",
                    claims.company_id
                );
                if (cnt[0].as<int>() >= max_users)
                    return json_error(403, fmt::format(
                        "Достигнут лимит пользователей ({}). Обновите подписку.", max_users));

                auto email     = (*body)["email"].get<std::string>();
                auto password  = (*body)["password"].get<std::string>();
                auto full_name = get_optional<std::string>(*body, "full_name");
                auto role      = body->contains("role") ? (*body)["role"].get<std::string>() : "user";

                if (password.size() < 8)
                    return json_error(400, "Пароль должен быть не менее 8 символов");

                // Уникальность email
                auto existing = txn.exec_params(
                    "SELECT id FROM users WHERE email=$1", email
                );
                if (!existing.empty())
                    return json_error(400, "Email уже занят");

                auto hashed = hash_password(password);

                auto row = txn.exec_params1(
                    "INSERT INTO users(company_id,email,hashed_password,full_name,role) "
                    "VALUES($1,$2,$3,NULLIF($4,''),$5) "
                    "RETURNING id,company_id,email,full_name,role,is_active,"
                    "notify_email,telegram_chat_id,created_at,last_login",
                    claims.company_id, email, hashed,
                    full_name.value_or(""), role
                );

                return json_created(user_to_json(row));
            });
        } catch (const std::exception& e) {
            spdlog::error("Create user: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── PATCH /api/v1/users/:id ───────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/users/<int>").methods(crow::HTTPMethod::Patch)
    ([&app](const crow::request& req, crow::response& res, int user_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        // Можно редактировать себя или (если admin) любого в компании
        if (claims.user_id != user_id && claims.role != "admin") {
            res = json_error(403, "Нет прав на редактирование этого пользователя");
            return;
        }

        auto body = parse_body(req);
        if (!body) { res = json_error(400, "Invalid JSON"); return; }

        // Менять role — только admin
        if (body->contains("role") && claims.role != "admin") {
            res = json_error(403, "Только администратор может менять роли");
            return;
        }

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT id FROM users WHERE id=$1 AND company_id=$2",
                    user_id, claims.company_id
                );
                if (rows.empty()) return json_error(404, "Пользователь не найден");

                std::vector<std::pair<std::string,std::string>> updates;
                if (body->contains("full_name"))        updates.push_back({"full_name",        (*body)["full_name"].is_null() ? "" : (*body)["full_name"].get<std::string>()});
                if (body->contains("role"))             updates.push_back({"role",             (*body)["role"].get<std::string>()});
                if (body->contains("notify_email"))     updates.push_back({"notify_email",     (*body)["notify_email"].get<bool>() ? "true" : "false"});
                if (body->contains("telegram_chat_id")) updates.push_back({"telegram_chat_id", (*body)["telegram_chat_id"].is_null() ? "" : (*body)["telegram_chat_id"].get<std::string>()});

                if (updates.empty()) return json_error(400, "Нет полей для обновления");

                std::string set_clause;
                for (auto& [col, val] : updates) {
                    if (!set_clause.empty()) set_clause += ",";
                    set_clause += fmt::format("{}={}", col, txn.quote(val));
                }

                std::string sql = "UPDATE users SET " + set_clause +
                    fmt::format(" WHERE id={} AND company_id={}"
                    " RETURNING id,company_id,email,full_name,role,is_active,"
                    "notify_email,telegram_chat_id,created_at,last_login",
                    txn.quote(user_id), txn.quote(claims.company_id));

                auto updated = txn.exec1(sql);
                return json_ok(user_to_json(updated));
            });
        } catch (const std::exception& e) {
            spdlog::error("Update user: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── DELETE /api/v1/users/:id  (деактивация, только admin) ────────────
    CROW_ROUTE(app, "/api/v1/users/<int>").methods(crow::HTTPMethod::Delete)
    ([&app](const crow::request& req, crow::response& res, int user_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_role(req, res, ctx, {"admin"})) return;
        auto& claims = *ctx.claims;

        if (claims.user_id == user_id) {
            res = json_error(400, "Нельзя деактивировать собственный аккаунт");
            return;
        }

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT id FROM users WHERE id=$1 AND company_id=$2",
                    user_id, claims.company_id
                );
                if (rows.empty()) return json_error(404, "Пользователь не найден");

                txn.exec_params(
                    "UPDATE users SET is_active=FALSE WHERE id=$1", user_id
                );
                return json_no_content();
            });
        } catch (const std::exception& e) {
            spdlog::error("Deactivate user: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });
}

} // namespace crm::api
