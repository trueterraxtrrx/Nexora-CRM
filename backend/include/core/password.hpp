#pragma once

#include <string>

namespace crm::core {

inline constexpr unsigned long BCRYPT_COST = 12;

std::string hash_password(const std::string& plain);
bool verify_password(const std::string& plain, const std::string& hash);

} // namespace crm::core
