#include "core/rate_limiter.hpp"
#include <spdlog/spdlog.h>

namespace crm::core {

RateLimiter& RateLimiter::instance() {
    static RateLimiter instance;
    return instance;
}

bool RateLimiter::allow_request(int company_id, int max_requests) {
    std::lock_guard lock(limits_mutex_);
    
    auto now = std::chrono::system_clock::now();
    auto& limit = limits_[company_id];
    
    // Reset window if time passed
    if (now - limit.window_start >= WINDOW_DURATION) {
        limit.request_count = 0;
        limit.window_start = now;
    }
    
    if (limit.request_count >= max_requests) {
        spdlog::warn("Rate limit exceeded for company {}", company_id);
        return false;
    }
    
    limit.request_count++;
    return true;
}

void RateLimiter::reset(int company_id) {
    std::lock_guard lock(limits_mutex_);
    limits_.erase(company_id);
}

} // namespace crm::core
// Project version: Nexora CRM V2.3
