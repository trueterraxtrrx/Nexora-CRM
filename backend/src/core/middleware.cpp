#include "core/middleware.hpp"
#include "utils/response.hpp"
#include <algorithm>

namespace crm::core {

namespace {

std::optional<std::string> bearer_token(const crow::request& req) {
    auto header = req.get_header_value("Authorization");
    constexpr std::string_view prefix = "Bearer ";
    if (header.rfind(prefix, 0) != 0) return std::nullopt;
    return header.substr(prefix.size());
}

} // namespace

void AuthMiddleware::before_handle(crow::request& req, crow::response&, context& ctx) {
    auto token = bearer_token(req);
    if (!token) return;

    try {
        ctx.claims = get_jwt().verify_token(*token);
    } catch (...) {
        ctx.claims.reset();
    }
}

bool require_auth(const crow::request&, crow::response& res, AuthMiddleware::context& ctx) {
    if (ctx.claims.has_value()) return true;
    res = crm::utils::json_error(401, "Unauthorized");
    return false;
}

bool require_role(
    const crow::request& req,
    crow::response& res,
    AuthMiddleware::context& ctx,
    const std::vector<std::string>& roles
) {
    if (!require_auth(req, res, ctx)) return false;

    auto& claims = *ctx.claims;
    const auto allowed = std::find(roles.begin(), roles.end(), claims.role);
    if (allowed != roles.end()) return true;

    res = crm::utils::json_error(403, "Forbidden");
    return false;
}

} // namespace crm::core
