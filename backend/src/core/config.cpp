#include "core/config.hpp"
#include <spdlog/spdlog.h>

namespace crm::core {

Config Config::load() {
    Config cfg;

    cfg.db_host      = get_env("DB_HOST", "localhost");
    cfg.db_port      = get_env("DB_PORT", "5432");
    cfg.db_name      = get_env("DB_NAME", "crm_db");
    cfg.db_user      = get_env("DB_USER", "crm_user");
    cfg.db_password  = get_env("DB_PASSWORD", "demo_only_change_me");
    cfg.db_pool_size = get_env_int("DB_POOL_SIZE", 10);

    cfg.jwt_secret       = require_env("JWT_SECRET");
    cfg.jwt_expiry_hours = get_env_int("JWT_EXPIRY_HOURS", 24);

    cfg.server_port    = get_env_int("PORT", 8000);
    cfg.server_threads = get_env_int("THREADS",
        static_cast<int>(std::thread::hardware_concurrency()));
    cfg.debug = get_env_bool("DEBUG", false);

    // Allowed origins (comma-separated)
    std::string origins_str = get_env("ALLOWED_ORIGINS",
        "http://localhost:3000,http://localhost:5173");
    std::stringstream ss(origins_str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) cfg.allowed_origins.push_back(token);
    }

    cfg.smtp_host     = get_env("SMTP_HOST", "smtp.gmail.com");
    cfg.smtp_port     = get_env_int("SMTP_PORT", 587);
    cfg.smtp_user     = get_env("SMTP_USER", "");
    cfg.smtp_password = get_env("SMTP_PASSWORD", "");

    cfg.telegram_bot_token = get_env("TELEGRAM_BOT_TOKEN", "");

    spdlog::info("Config loaded: port={}, threads={}, debug={}",
        cfg.server_port, cfg.server_threads, cfg.debug);

    return cfg;
}

Config& get_config() {
    static Config instance = Config::load();
    return instance;
}

} // namespace crm::core
