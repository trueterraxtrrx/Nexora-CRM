#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <string>

namespace crm::metrics {

class PrometheusMetrics {
public:
    static PrometheusMetrics& instance();

    void increment_requests();
    void increment_requests_by_endpoint(const std::string& endpoint);
    void increment_errors();
    void increment_db_queries();
    void record_request_latency(double ms);
    void record_db_query_latency(double ms);
    void set_active_connections(int count);
    void set_db_pool_size(int size);
    std::string export_metrics();

private:
    std::atomic<long long> total_requests_{0};
    std::atomic<long long> total_errors_{0};
    std::atomic<long long> total_db_queries_{0};
    std::atomic<int> active_connections_{0};
    std::atomic<int> db_pool_size_{0};
    std::mutex counters_mutex_;
    std::map<std::string, std::atomic<long long>> endpoint_counters_;
};

} // namespace crm::metrics
