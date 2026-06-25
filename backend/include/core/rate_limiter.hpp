#pragma once

#include <chrono>
#include <map>
#include <mutex>

namespace crm::core {

class RateLimiter {
public:
    static RateLimiter& instance();
    bool allow_request(int company_id, int max_requests);
    void reset(int company_id);

private:
    struct Limit {
        int request_count = 0;
        std::chrono::system_clock::time_point window_start = std::chrono::system_clock::now();
    };

    static constexpr auto WINDOW_DURATION = std::chrono::minutes(1);

    std::mutex limits_mutex_;
    std::map<int, Limit> limits_;
};

} // namespace crm::core
