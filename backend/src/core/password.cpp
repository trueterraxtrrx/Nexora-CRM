#include "core/password.hpp"

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <array>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

#ifndef _WIN32
extern "C" {
char* crypt_gensalt_rn(const char* prefix, unsigned long count,
                       const char* input, int size,
                       char* output, int output_size);
char* crypt_rn(const char* key, const char* setting, void* data, int size);
}
#endif

namespace crm::core {

#ifdef _WIN32
namespace {
constexpr int PBKDF2_ITERATIONS = 210000;
constexpr int SALT_BYTES = 16;
constexpr int KEY_BYTES = 32;

std::string to_hex(const unsigned char* data, std::size_t size) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < size; ++i) {
        out << std::setw(2) << static_cast<int>(data[i]);
    }
    return out.str();
}

std::vector<unsigned char> from_hex(const std::string& hex) {
    if (hex.size() % 2 != 0) return {};
    std::vector<unsigned char> bytes(hex.size() / 2);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<unsigned char>(std::stoul(hex.substr(i * 2, 2), nullptr, 16));
    }
    return bytes;
}
} // namespace
#endif

std::string hash_password(const std::string& plain) {
#ifdef _WIN32
    std::array<unsigned char, SALT_BYTES> salt{};
    std::array<unsigned char, KEY_BYTES> key{};
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }
    if (PKCS5_PBKDF2_HMAC(
            plain.c_str(), static_cast<int>(plain.size()),
            salt.data(), static_cast<int>(salt.size()),
            PBKDF2_ITERATIONS, EVP_sha256(),
            static_cast<int>(key.size()), key.data()) != 1) {
        throw std::runtime_error("PKCS5_PBKDF2_HMAC failed");
    }
    return "pbkdf2_sha256$" + std::to_string(PBKDF2_ITERATIONS) + "$" +
           to_hex(salt.data(), salt.size()) + "$" +
           to_hex(key.data(), key.size());
#else
    unsigned char rand_buf[16];
    if (RAND_bytes(rand_buf, sizeof(rand_buf)) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }

    char salt[64] = {};
    char* result_salt = crypt_gensalt_rn(
        "$2b$", BCRYPT_COST,
        reinterpret_cast<const char*>(rand_buf), sizeof(rand_buf),
        salt, sizeof(salt));
    if (!result_salt) throw std::runtime_error("crypt_gensalt_rn failed");

    std::array<char, 256> output{};
    char* hashed = crypt_rn(plain.c_str(), salt, output.data(), static_cast<int>(output.size()));
    if (!hashed) throw std::runtime_error("crypt_rn failed");

    return std::string(hashed);
#endif
}

bool verify_password(const std::string& plain, const std::string& hash) {
#ifdef _WIN32
    const std::string prefix = "pbkdf2_sha256$";
    if (hash.rfind(prefix, 0) != 0) return false;
    const auto iter_end = hash.find('$', prefix.size());
    if (iter_end == std::string::npos) return false;
    const auto salt_end = hash.find('$', iter_end + 1);
    if (salt_end == std::string::npos) return false;

    const int iterations = std::stoi(hash.substr(prefix.size(), iter_end - prefix.size()));
    const auto salt = from_hex(hash.substr(iter_end + 1, salt_end - iter_end - 1));
    const auto expected = from_hex(hash.substr(salt_end + 1));
    if (salt.empty() || expected.empty()) return false;

    std::vector<unsigned char> actual(expected.size());
    if (PKCS5_PBKDF2_HMAC(
            plain.c_str(), static_cast<int>(plain.size()),
            salt.data(), static_cast<int>(salt.size()),
            iterations, EVP_sha256(),
            static_cast<int>(actual.size()), actual.data()) != 1) {
        return false;
    }
    return CRYPTO_memcmp(actual.data(), expected.data(), expected.size()) == 0;
#else
    std::array<char, 256> output{};
    char* result = crypt_rn(plain.c_str(), hash.c_str(), output.data(), static_cast<int>(output.size()));
    if (!result) return false;
    return hash == std::string(result);
#endif
}

} // namespace crm::core
// Project version: Nexora CRM V2.7


