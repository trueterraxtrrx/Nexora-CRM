#pragma once
#include <string>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace crm::core {

struct JwtClaims {
    int  user_id;
    int  company_id;
    std::string role;
    std::chrono::system_clock::time_point expires_at;
};

class JwtService {
public:
    explicit JwtService(std::string secret, int expiry_hours = 24)
        : secret_(std::move(secret)), expiry_hours_(expiry_hours) {}

    // Создаёт подписанный JWT токен
    [[nodiscard]] std::string create_token(
        int user_id,
        int company_id,
        const std::string& role
    ) const;

    // Валидирует и декодирует токен
    // Возвращает nullopt если токен невалиден/истёк
    [[nodiscard]] std::optional<JwtClaims> verify_token(const std::string& token) const;

private:
    std::string base64url_encode(const std::string& data) const;
    std::string base64url_decode(const std::string& data) const;
    std::string hmac_sha256(const std::string& data, const std::string& key) const;

    std::string secret_;
    int expiry_hours_;
};

// Global singleton
JwtService& get_jwt();
void init_jwt(const std::string& secret, int expiry_hours);

} // namespace crm::core
