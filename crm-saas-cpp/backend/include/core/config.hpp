#pragma once
#include <string>
#include <vector>
#include <cstdlib>
#include <stdexcept>
#include <fmt/format.h>

namespace crm::core {

struct Config {
    // ── Database ────────────────────────────────────────────────
    std::string db_host;
    std::string db_port;
    std::string db_name;
    std::string db_user;
    std::string db_password;
    int         db_pool_size;

    // ── JWT ─────────────────────────────────────────────────────
    std::string jwt_secret;
    int         jwt_expiry_hours;

    // ── Server ──────────────────────────────────────────────────
    int         server_port;
    int         server_threads;
    bool        debug;

    // ── CORS ────────────────────────────────────────────────────
    std::vector<std::string> allowed_origins;

    // ── Email ────────────────────────────────────────────────────
    std::string smtp_host;
    int         smtp_port;
    std::string smtp_user;
    std::string smtp_password;

    // ── Telegram ────────────────────────────────────────────────
    std::string telegram_bot_token;

    // Build PostgreSQL connection string
    [[nodiscard]] std::string pg_conn_string() const {
        return fmt::format(
            "host={} port={} dbname={} user={} password={} "
            "connect_timeout=10 application_name=crm_backend",
            db_host, db_port, db_name, db_user, db_password
        );
    }

    static Config load();

private:
    static std::string require_env(const char* name) {
        const char* val = std::getenv(name);
        if (!val || std::string(val).empty())
            throw std::runtime_error(fmt::format("Required env var '{}' is not set", name));
        return val;
    }

    static std::string get_env(const char* name, std::string default_val = "") {
        const char* val = std::getenv(name);
        return (val && *val) ? val : default_val;
    }

    static int get_env_int(const char* name, int default_val) {
        const char* val = std::getenv(name);
        if (!val || !*val) return default_val;
        try { return std::stoi(val); }
        catch (...) { return default_val; }
    }

    static bool get_env_bool(const char* name, bool default_val = false) {
        const char* val = std::getenv(name);
        if (!val || !*val) return default_val;
        std::string s(val);
        return s == "1" || s == "true" || s == "True" || s == "TRUE";
    }
};

// Global singleton — инициализируется один раз при старте
Config& get_config();

} // namespace crm::core
