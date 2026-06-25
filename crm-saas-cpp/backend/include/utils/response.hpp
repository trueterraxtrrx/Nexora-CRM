#pragma once
#include <crow.h>
#include <nlohmann/json.hpp>
#include <string>

namespace crm::utils {

inline crow::response json_ok(const nlohmann::json& body, int code = 200) {
    crow::response res(code, body.dump());
    res.add_header("Content-Type", "application/json");
    return res;
}

inline crow::response json_created(const nlohmann::json& body) {
    return json_ok(body, 201);
}

inline crow::response json_error(int code, const std::string& detail) {
    return json_ok({{"detail", detail}}, code);
}

inline crow::response json_no_content() {
    crow::response res(204);
    return res;
}

// Парсит тело запроса как JSON, возвращает nullopt при ошибке
inline std::optional<nlohmann::json> parse_body(const crow::request& req) {
    try {
        if (req.body.empty()) return std::nullopt;
        return nlohmann::json::parse(req.body);
    } catch (...) {
        return std::nullopt;
    }
}

// Получает optional<string> из JSON поля
template<typename T>
inline std::optional<T> get_optional(const nlohmann::json& j, const std::string& key) {
    if (!j.contains(key) || j[key].is_null()) return std::nullopt;
    try { return j[key].get<T>(); }
    catch (...) { return std::nullopt; }
}

} // namespace crm::utils
