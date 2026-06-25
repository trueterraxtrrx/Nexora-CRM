#pragma once

#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#ifdef HAVE_REDIS
#include <hiredis/hiredis.h>
#else
struct redisContext;
struct redisReply;
#endif

namespace crm::cache {

class RedisCache {
public:
    static RedisCache& instance();
    ~RedisCache();

    bool connect(const std::string& host, int port);
    void disconnect();
    bool set(const std::string& key, const std::string& value, int ttl_seconds);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    void flush();
    bool set_json(const std::string& key, const nlohmann::json& value, int ttl);
    std::optional<nlohmann::json> get_json(const std::string& key);

private:
    redisContext* context_ = nullptr;
};

} // namespace crm::cache
