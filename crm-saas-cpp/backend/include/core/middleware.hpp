#pragma once
#include <crow.h>
#include <functional>
#include <string>
#include <optional>
#include "core/jwt.hpp"

namespace crm::core {

// ─── Auth Middleware ─────────────────────────────────────────────────────────
// Crow middleware: извлекает и проверяет JWT из заголовка Authorization.
// Кладёт JwtClaims в контекст запроса.

struct AuthMiddleware {
    struct context {
        std::optional<JwtClaims> claims;
        bool authenticated() const { return claims.has_value(); }
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        auto auth_header = req.get_header_value("Authorization");

        if (auth_header.substr(0, 7) != "Bearer ") {
            ctx.claims = std::nullopt;
            return;
        }

        std::string token = auth_header.substr(7);
        ctx.claims = get_jwt().verify_token(token);
    }

    void after_handle(crow::request&, crow::response&, context&) {}
};

// ─── CORS Middleware ─────────────────────────────────────────────────────────

struct CorsMiddleware {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context&) {
        // Preflight
        if (req.method == crow::HTTPMethod::Options) {
            res.add_header("Access-Control-Allow-Origin",  "*");
            res.add_header("Access-Control-Allow-Methods", "GET,POST,PUT,PATCH,DELETE,OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type,Authorization");
            res.add_header("Access-Control-Max-Age",       "86400");
            res.code = 204;
            res.end();
        }
    }

    void after_handle(crow::request&, crow::response& res, context&) {
        res.add_header("Access-Control-Allow-Origin", "*");
    }
};

// ─── Guards (хелперы для роутеров) ──────────────────────────────────────────

using AppType = crow::App<CorsMiddleware, AuthMiddleware>;

// Требует авторизации — возвращает 401 если нет токена
inline bool require_auth(
    const crow::request& req,
    crow::response& res,
    AppType::context& ctx
) {
    auto& auth_ctx = ctx.get<AuthMiddleware>();
    if (!auth_ctx.authenticated()) {
        res.code = 401;
        res.write(R"({"detail":"Требуется авторизация"})");
        res.end();
        return false;
    }
    return true;
}

// Требует определённую роль
inline bool require_role(
    const crow::request& req,
    crow::response& res,
    AppType::context& ctx,
    std::initializer_list<std::string> roles
) {
    if (!require_auth(req, res, ctx)) return false;
    auto& auth_ctx = ctx.get<AuthMiddleware>();
    const auto& role = auth_ctx.claims->role;
    for (const auto& r : roles) {
        if (role == r) return true;
    }
    res.code = 403;
    res.write(R"({"detail":"Недостаточно прав"})");
    res.end();
    return false;
}

} // namespace crm::core
