#include "core/password.hpp"
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <stdexcept>
#include <cstring>
#include <array>

// Используем crypt_blowfish (libbcrypt)
extern "C" {
    char* crypt_gensalt_rn(const char* prefix, unsigned long count,
                            const char* input, int size,
                            char* output, int output_size);
    char* crypt_rn(const char* key, const char* setting,
                   void* data, int size);
}

namespace crm::core {

std::string hash_password(const std::string& plain) {
    // Генерируем криптографически стойкую соль
    unsigned char rand_buf[16];
    if (RAND_bytes(rand_buf, sizeof(rand_buf)) != 1)
        throw std::runtime_error("RAND_bytes failed");

    // Генерируем bcrypt соль
    char salt[64] = {};
    char* result_salt = crypt_gensalt_rn(
        "$2b$", BCRYPT_COST,
        reinterpret_cast<const char*>(rand_buf), sizeof(rand_buf),
        salt, sizeof(salt)
    );
    if (!result_salt) throw std::runtime_error("crypt_gensalt_rn failed");

    // Хэшируем
    std::array<char, 256> output{};
    char* hashed = crypt_rn(plain.c_str(), salt, output.data(), static_cast<int>(output.size()));
    if (!hashed) throw std::runtime_error("crypt_rn failed");

    return std::string(hashed);
}

bool verify_password(const std::string& plain, const std::string& hash) {
    std::array<char, 256> output{};
    char* result = crypt_rn(plain.c_str(), hash.c_str(), output.data(), static_cast<int>(output.size()));
    if (!result) return false;
    return hash == std::string(result);
}

} // namespace crm::core
// Project version: Nexora CRM V2.3
