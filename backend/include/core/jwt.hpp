#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <nlohmann/json.hpp>

namespace crm::core {

struct JwtClaims {
    int user_id{};
    int company_id{};
    std::string role;
    std::chrono::system_clock::time_point expires_at;
};

class JwtService {
public:
    JwtService(std::string secret, int expiry_hours)
        : secret_(std::move(secret)), expiry_hours_(expiry_hours) {}

    std::string create_token(int user_id, int company_id, const std::string& role) const;
    std::optional<JwtClaims> verify_token(const std::string& token) const;

private:
    std::string base64url_encode(const std::string& data) const;
    std::string base64url_decode(const std::string& input) const;
    std::string hmac_sha256(const std::string& data, const std::string& key) const;

    std::string secret_;
    int expiry_hours_;
};

void init_jwt(const std::string& secret, int expiry_hours);
JwtService& get_jwt();

} // namespace crm::core
