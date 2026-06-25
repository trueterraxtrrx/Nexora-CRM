#pragma once

#include <optional>
#include <string>
#include <vector>
#include <crow.h>
#include "core/jwt.hpp"

namespace crm::core {

struct AuthMiddleware {
    struct context {
        std::optional<JwtClaims> claims;
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request&, crow::response&, context&) {}
};

using AppType = crow::App<AuthMiddleware>;

bool require_auth(const crow::request& req, crow::response& res, AuthMiddleware::context& ctx);
bool require_role(
    const crow::request& req,
    crow::response& res,
    AuthMiddleware::context& ctx,
    const std::vector<std::string>& roles
);

} // namespace crm::core
