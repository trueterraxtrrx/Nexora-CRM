#include "core/jwt.hpp"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <ctime>

namespace crm::core {

static std::unique_ptr<JwtService> g_jwt;

JwtService& get_jwt() {
    if (!g_jwt) throw std::runtime_error("JWT not initialized");
    return *g_jwt;
}

void init_jwt(const std::string& secret, int expiry_hours) {
    g_jwt = std::make_unique<JwtService>(secret, expiry_hours);
}

// ─── Base64URL ───────────────────────────────────────────────────────────────

std::string JwtService::base64url_encode(const std::string& data) const {
    static const char* chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;
    result.reserve(((data.size() + 2) / 3) * 4);

    for (size_t i = 0; i < data.size(); i += 3) {
        unsigned char b0 = static_cast<unsigned char>(data[i]);
        unsigned char b1 = (i + 1 < data.size()) ? static_cast<unsigned char>(data[i+1]) : 0;
        unsigned char b2 = (i + 2 < data.size()) ? static_cast<unsigned char>(data[i+2]) : 0;

        result += chars[b0 >> 2];
        result += chars[((b0 & 0x3) << 4) | (b1 >> 4)];
        result += (i + 1 < data.size()) ? chars[((b1 & 0xF) << 2) | (b2 >> 6)] : '=';
        result += (i + 2 < data.size()) ? chars[b2 & 0x3F] : '=';
    }

    // Base64 → Base64URL: заменяем + → - и / → _ и убираем =
    for (auto& c : result) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    while (!result.empty() && result.back() == '=') result.pop_back();

    return result;
}

std::string JwtService::base64url_decode(const std::string& input) const {
    std::string data = input;
    // Base64URL → Base64
    for (auto& c : data) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }
    // Добавляем padding
    while (data.size() % 4 != 0) data += '=';

    static const int decode_table[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };

    std::string result;
    for (size_t i = 0; i < data.size(); i += 4) {
        int v = (decode_table[(unsigned char)data[i]]   << 18) |
                (decode_table[(unsigned char)data[i+1]] << 12) |
                (decode_table[(unsigned char)data[i+2]] <<  6) |
                (decode_table[(unsigned char)data[i+3]]);
        result += static_cast<char>((v >> 16) & 0xFF);
        if (data[i+2] != '=') result += static_cast<char>((v >>  8) & 0xFF);
        if (data[i+3] != '=') result += static_cast<char>( v        & 0xFF);
    }
    return result;
}

// ─── HMAC-SHA256 ─────────────────────────────────────────────────────────────

std::string JwtService::hmac_sha256(const std::string& data, const std::string& key) const {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int  digest_len = 0;

    HMAC(EVP_sha256(),
         key.data(),  static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char*>(data.data()),
         data.size(),
         digest, &digest_len);

    return std::string(reinterpret_cast<char*>(digest), digest_len);
}

// ─── Token creation ──────────────────────────────────────────────────────────

std::string JwtService::create_token(int user_id, int company_id, const std::string& role) const {
    using namespace std::chrono;

    // Header
    nlohmann::json header = {{"alg","HS256"}, {"typ","JWT"}};
    std::string header_b64 = base64url_encode(header.dump());

    // Payload
    auto now = system_clock::now();
    auto exp = now + hours(expiry_hours_);
    int64_t iat = duration_cast<seconds>(now.time_since_epoch()).count();
    int64_t exp_ts = duration_cast<seconds>(exp.time_since_epoch()).count();

    nlohmann::json payload = {
        {"sub",        user_id},
        {"company_id", company_id},
        {"role",       role},
        {"iat",        iat},
        {"exp",        exp_ts}
    };
    std::string payload_b64 = base64url_encode(payload.dump());

    // Signature
    std::string signing_input = header_b64 + "." + payload_b64;
    std::string sig = hmac_sha256(signing_input, secret_);
    std::string sig_b64 = base64url_encode(sig);

    return signing_input + "." + sig_b64;
}

// ─── Token verification ──────────────────────────────────────────────────────

std::optional<JwtClaims> JwtService::verify_token(const std::string& token) const {
    // Разбиваем на 3 части
    auto dot1 = token.find('.');
    if (dot1 == std::string::npos) return std::nullopt;
    auto dot2 = token.find('.', dot1 + 1);
    if (dot2 == std::string::npos) return std::nullopt;

    std::string header_b64  = token.substr(0, dot1);
    std::string payload_b64 = token.substr(dot1 + 1, dot2 - dot1 - 1);
    std::string sig_b64     = token.substr(dot2 + 1);

    // Верифицируем подпись
    std::string signing_input = header_b64 + "." + payload_b64;
    std::string expected_sig  = base64url_encode(hmac_sha256(signing_input, secret_));

    if (sig_b64 != expected_sig) {
        spdlog::debug("JWT signature mismatch");
        return std::nullopt;
    }

    // Декодируем payload
    try {
        auto payload_json = nlohmann::json::parse(base64url_decode(payload_b64));

        int64_t exp_ts = payload_json["exp"].get<int64_t>();
        auto now_ts = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        if (now_ts > exp_ts) {
            spdlog::debug("JWT token expired");
            return std::nullopt;
        }

        JwtClaims claims;
        claims.user_id    = payload_json["sub"].get<int>();
        claims.company_id = payload_json["company_id"].get<int>();
        claims.role       = payload_json["role"].get<std::string>();
        claims.expires_at = std::chrono::system_clock::from_time_t(exp_ts);

        return claims;

    } catch (const std::exception& e) {
        spdlog::warn("JWT parse error: {}", e.what());
        return std::nullopt;
    }
}

} // namespace crm::core
// Project version: Nexora CRM V2.3
