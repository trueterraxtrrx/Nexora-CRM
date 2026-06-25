#pragma once

#include <optional>
#include <string>
#include <crow.h>
#include <nlohmann/json.hpp>

namespace crm::utils {

inline crow::response json_response(int status, const nlohmann::json& body) {
    crow::response res(status, body.dump());
    res.add_header("Content-Type", "application/json");
    return res;
}

inline crow::response json_ok(const nlohmann::json& body) {
    return json_response(200, body);
}

inline crow::response json_created(const nlohmann::json& body) {
    return json_response(201, body);
}

inline crow::response json_no_content() {
    return crow::response(204);
}

inline crow::response json_error(int status, const std::string& detail) {
    return json_response(status, {{"detail", detail}});
}

inline std::optional<nlohmann::json> parse_body(const crow::request& req) {
    try {
        if (req.body.empty()) return nlohmann::json::object();
        return nlohmann::json::parse(req.body);
    } catch (...) {
        return std::nullopt;
    }
}

template <typename T>
std::optional<T> get_optional(const nlohmann::json& body, const std::string& key) {
    if (!body.contains(key) || body.at(key).is_null()) return std::nullopt;
    return body.at(key).get<T>();
}

} // namespace crm::utils
