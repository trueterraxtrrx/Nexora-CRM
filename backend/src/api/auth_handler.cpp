#include "api/auth_handler.hpp"
#include "core/database.hpp"
#include "core/jwt.hpp"
#include "core/password.hpp"
#include "utils/response.hpp"
#include "utils/slugify.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <utility>

namespace crm::api {

using namespace crm::core;
using namespace crm::utils;
using json = nlohmann::json;

void register_auth_routes(AppType& app) {

    // ── POST /api/v1/auth/register ─────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/auth/register").methods(crow::HTTPMethod::Post)
    ([&app](const crow::request& req) -> crow::response {
        auto body = parse_body(req);
        if (!body) return json_error(400, "Invalid JSON");

        // Валидация
        if (!body->contains("email") || !body->contains("password") ||
            !body->contains("full_name") || !body->contains("company_name"))
            return json_error(400, "Все поля обязательны: email, password, full_name, company_name");

        auto email        = (*body)["email"].get<std::string>();
        auto password     = (*body)["password"].get<std::string>();
        auto full_name    = (*body)["full_name"].get<std::string>();
        auto company_name = (*body)["company_name"].get<std::string>();

        if (password.size() < 8)
            return json_error(400, "Пароль должен быть не менее 8 символов");

        try {
            return get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                // Проверяем уникальность email
                auto existing = txn.exec_params(
                    "SELECT id FROM users WHERE email=$1", email
                );
                if (!existing.empty())
                    return json_error(400, "Пользователь с таким email уже существует");

                // Уникальный slug компании
                std::string base_slug = slugify(company_name);
                std::string slug = base_slug;
                int counter = 1;
                while (true) {
                    auto slug_check = txn.exec_params(
                        "SELECT id FROM companies WHERE slug=$1", slug
                    );
                    if (slug_check.empty()) break;
                    slug = base_slug + "-" + std::to_string(counter++);
                }

                // Создаём компанию
                auto comp = txn.exec_params1(
                    "INSERT INTO companies(name,slug,plan,max_users,max_clients) "
                    "VALUES($1,$2,'free',3,50) RETURNING id,name,slug,plan,created_at",
                    company_name, slug
                );
                int company_id = comp["id"].as<int>();

                // Хэшируем пароль
                std::string hashed = hash_password(password);

                // Создаём пользователя-admin
                auto user = txn.exec_params1(
                    "INSERT INTO users(company_id,email,hashed_password,full_name,role) "
                    "VALUES($1,$2,$3,$4,'admin') "
                    "RETURNING id,company_id,email,full_name,role,created_at",
                    company_id, email, hashed, full_name
                );

                json result = {
                    {"id",         user["id"].as<int>()},
                    {"company_id", user["company_id"].as<int>()},
                    {"email",      user["email"].as<std::string>()},
                    {"full_name",  user["full_name"].is_null() ? "" : user["full_name"].as<std::string>()},
                    {"role",       user["role"].as<std::string>()},
                    {"is_active",  true},
                    {"created_at", user["created_at"].as<std::string>()}
                };
                return json_created(result);
            });
        } catch (const std::exception& e) {
            spdlog::error("Register error: {}", e.what());
            return json_error(500, "Внутренняя ошибка сервера");
        }
    });

    // ── POST /api/v1/auth/login ────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/auth/login").methods(crow::HTTPMethod::Post)
    ([&app](const crow::request& req) -> crow::response {
        auto body = parse_body(req);
        if (!body) return json_error(400, "Invalid JSON");

        if (!body->contains("email") || !body->contains("password"))
            return json_error(400, "email и password обязательны");

        auto email    = (*body)["email"].get<std::string>();
        auto password = (*body)["password"].get<std::string>();

        try {
            return get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT id,company_id,hashed_password,role,is_active "
                    "FROM users WHERE email=$1",
                    email
                );

                if (rows.empty() || !rows[0]["is_active"].as<bool>())
                    return json_error(401, "Неверный email или пароль");

                auto row = rows[0];
                if (!verify_password(password, row["hashed_password"].as<std::string>()))
                    return json_error(401, "Неверный email или пароль");

                int user_id    = row["id"].as<int>();
                int company_id = row["company_id"].as<int>();
                auto role      = row["role"].as<std::string>();

                // Обновляем last_login
                txn.exec_params(
                    "UPDATE users SET last_login=NOW() WHERE id=$1", user_id
                );

                auto token = get_jwt().create_token(user_id, company_id, role);
                return json_ok({
                    {"access_token", token},
                    {"token_type",   "bearer"}
                });
            });
        } catch (const std::exception& e) {
            spdlog::error("Login error: {}", e.what());
            return json_error(500, "Внутренняя ошибка сервера");
        }
    });

    // ── GET /api/v1/auth/me ────────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/auth/me").methods(crow::HTTPMethod::Get)
    ([&app](const crow::request& req, crow::response& res) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        try {
            auto result = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto row = txn.exec_params1(
                    "SELECT id,company_id,email,full_name,role,is_active,"
                    "avatar_url,notify_email,telegram_chat_id,created_at,last_login "
                    "FROM users WHERE id=$1 AND is_active=TRUE",
                    claims.user_id
                );

                return json_ok({
                    {"id",               row["id"].as<int>()},
                    {"company_id",       row["company_id"].as<int>()},
                    {"email",            row["email"].as<std::string>()},
                    {"full_name",        row["full_name"].is_null() ? nullptr : json(row["full_name"].as<std::string>())},
                    {"role",             row["role"].as<std::string>()},
                    {"is_active",        row["is_active"].as<bool>()},
                    {"notify_email",     row["notify_email"].as<bool>()},
                    {"created_at",       row["created_at"].as<std::string>()},
                    {"last_login",       row["last_login"].is_null() ? nullptr : json(row["last_login"].as<std::string>())}
                });
            });
            res = std::move(result);
        } catch (const pqxx::unexpected_rows&) {
            res = json_error(401, "Пользователь не найден");
        } catch (const std::exception& e) {
            spdlog::error("Me error: {}", e.what());
            res = json_error(500, "Внутренняя ошибка сервера");
        }
    });
}

} // namespace crm::api
