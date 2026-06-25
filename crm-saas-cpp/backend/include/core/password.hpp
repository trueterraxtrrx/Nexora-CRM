#pragma once
#include <string>

namespace crm::core {

// bcrypt cost factor (2^cost итераций)
constexpr int BCRYPT_COST = 12;

// Хэширует пароль через bcrypt
[[nodiscard]] std::string hash_password(const std::string& plain);

// Верифицирует пароль против хэша
[[nodiscard]] bool verify_password(const std::string& plain, const std::string& hash);

} // namespace crm::core
