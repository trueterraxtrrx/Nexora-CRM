#include <crow.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <fmt/format.h>

#include "core/config.hpp"
#include "core/database.hpp"
#include "core/jwt.hpp"
#include "core/middleware.hpp"
#include "api/auth_handler.hpp"
#include "api/clients_handler.hpp"
#include "api/tasks_handler.hpp"
#include "api/finance_handler.hpp"
#include "api/users_handler.hpp"

int main() {
    // ── Logging ───────────────────────────────────────────────────────────
    auto logger = spdlog::stdout_color_mt("crm");
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");

    // ── Config ────────────────────────────────────────────────────────────
    auto& cfg = crm::core::get_config();
    spdlog::set_level(cfg.debug ? spdlog::level::debug : spdlog::level::info);

    // ── Database ──────────────────────────────────────────────────────────
    spdlog::info("Connecting to PostgreSQL...");
    crm::core::init_db(cfg.pg_conn_string(), cfg.db_pool_size);

    // Миграции — выполняем через отдельное соединение при старте
    {
        auto conn = crm::core::get_db().acquire();
        crm::core::run_migrations(*conn);
    }

    // ── JWT ───────────────────────────────────────────────────────────────
    crm::core::init_jwt(cfg.jwt_secret, cfg.jwt_expiry_hours);

    // ── Crow App ──────────────────────────────────────────────────────────
    crm::core::AppType app;

    // Crow logging — только в debug режиме
    if (!cfg.debug) {
        app.loglevel(crow::LogLevel::Warning);
    }

    // ── Routes ────────────────────────────────────────────────────────────
    crm::api::register_auth_routes(app);
    crm::api::register_users_routes(app);
    crm::api::register_clients_routes(app);
    crm::api::register_tasks_routes(app);
    crm::api::register_finance_routes(app);

    // ── Health check ──────────────────────────────────────────────────────
    CROW_ROUTE(app, "/api/health")
    ([]() {
        return crow::response(200, R"({"status":"ok","version":"2.2.0"})");
    });

    // 404 handler
    CROW_CATCHALL_ROUTE(app)
    ([](const crow::request& req) {
        crow::response res(404, R"({"detail":"Endpoint not found"})");
        res.add_header("Content-Type", "application/json");
        return res;
    });

    // ── Start ─────────────────────────────────────────────────────────────
    spdlog::info("Starting CRM Backend on port {}", cfg.server_port);
    spdlog::info("Threads: {}", cfg.server_threads);
    spdlog::info("API docs: http://localhost:{}/api/health", cfg.server_port);

    app
        .port(cfg.server_port)
        .concurrency(cfg.server_threads)
        .run();

    return 0;
}
