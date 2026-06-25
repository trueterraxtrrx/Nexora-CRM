#include "cache/redis_cache.hpp"
#include <spdlog/spdlog.h>

namespace crm::cache {

RedisCache& RedisCache::instance() {
    static RedisCache instance;
    return instance;
}

bool RedisCache::connect(const std::string& host, int port) {
#ifdef HAVE_REDIS
    context_ = redisConnect(host.c_str(), port);
    if (!context_ || context_->err) {
        spdlog::error("Redis connection failed");
        return false;
    }
    spdlog::info("Redis connected at {}:{}", host, port);
    return true;
#else
    spdlog::warn("Redis support was not built; cache disabled for {}:{}", host, port);
    return false;
#endif
}

void RedisCache::disconnect() {
#ifdef HAVE_REDIS
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }
#endif
}

RedisCache::~RedisCache() { disconnect(); }

bool RedisCache::set(const std::string& key, const std::string& value, int ttl_seconds) {
#ifdef HAVE_REDIS
    if (!context_) return false;
    redisReply* r = (redisReply*)redisCommand(context_, "SETEX %s %d %s", key.c_str(), ttl_seconds, value.c_str());
    bool ok = r && r->type == REDIS_REPLY_STATUS;
    if (r) freeReplyObject(r);
    return ok;
#else
    (void)key;
    (void)value;
    (void)ttl_seconds;
    return false;
#endif
}

std::optional<std::string> RedisCache::get(const std::string& key) {
#ifdef HAVE_REDIS
    if (!context_) return std::nullopt;
    redisReply* r = (redisReply*)redisCommand(context_, "GET %s", key.c_str());
    std::optional<std::string> result;
    if (r && r->type == REDIS_REPLY_STRING) result = std::string(r->str, r->len);
    if (r) freeReplyObject(r);
    return result;
#else
    (void)key;
    return std::nullopt;
#endif
}

bool RedisCache::del(const std::string& key) {
#ifdef HAVE_REDIS
    if (!context_) return false;
    redisReply* r = (redisReply*)redisCommand(context_, "DEL %s", key.c_str());
    bool ok = r && r->integer > 0;
    if (r) freeReplyObject(r);
    return ok;
#else
    (void)key;
    return false;
#endif
}

void RedisCache::flush() {
#ifdef HAVE_REDIS
    if (context_) redisCommand(context_, "FLUSHDB");
#endif
}

bool RedisCache::set_json(const std::string& key, const nlohmann::json& value, int ttl) {
    return set(key, value.dump(), ttl);
}

std::optional<nlohmann::json> RedisCache::get_json(const std::string& key) {
    auto val = get(key);
    if (!val) return std::nullopt;
    try { return nlohmann::json::parse(*val); }
    catch (...) { return std::nullopt; }
}

} // namespace crm::cache
