#pragma once

#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace crm::core {

struct Config {
    std::string db_host;
    std::string db_port;
    std::string db_name;
    std::string db_user;
    std::string db_password;
    int db_pool_size{};

    std::string jwt_secret;
    int jwt_expiry_hours{};

    int server_port{};
    int server_threads{};
    bool debug{};
    std::vector<std::string> allowed_origins;

    std::string smtp_host;
    int smtp_port{};
    std::string smtp_user;
    std::string smtp_password;

    std::string telegram_bot_token;

    static Config load();

    std::string pg_conn_string() const {
        return "host=" + db_host + " port=" + db_port + " dbname=" + db_name +
               " user=" + db_user + " password=" + db_password;
    }
};

inline std::string get_env(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : fallback;
}

inline std::string require_env(const char* name) {
    const char* value = std::getenv(name);
    if (!value || std::string(value).empty()) {
        throw std::runtime_error(std::string("Missing required environment variable: ") + name);
    }
    return value;
}

inline int get_env_int(const char* name, int fallback) {
    const char* value = std::getenv(name);
    return value ? std::stoi(value) : fallback;
}

inline bool get_env_bool(const char* name, bool fallback) {
    const char* value = std::getenv(name);
    if (!value) return fallback;
    std::string normalized(value);
    return normalized == "1" || normalized == "true" || normalized == "TRUE" || normalized == "yes";
}

Config& get_config();

} // namespace crm::core
