#include "metrics/prometheus.hpp"
#include <sstream>
#include <fmt/format.h>

namespace crm::metrics {

PrometheusMetrics& PrometheusMetrics::instance() {
    static PrometheusMetrics instance;
    return instance;
}

void PrometheusMetrics::increment_requests() { ++total_requests_; }
void PrometheusMetrics::increment_requests_by_endpoint(const std::string& endpoint) {
    std::lock_guard lock(counters_mutex_);
    endpoint_counters_[endpoint]++;
}
void PrometheusMetrics::increment_errors() { ++total_errors_; }
void PrometheusMetrics::increment_db_queries() { ++total_db_queries_; }
void PrometheusMetrics::record_request_latency(double ms) {}
void PrometheusMetrics::record_db_query_latency(double ms) {}
void PrometheusMetrics::set_active_connections(int count) { active_connections_ = count; }
void PrometheusMetrics::set_db_pool_size(int size) { db_pool_size_ = size; }

std::string PrometheusMetrics::export_metrics() {
    std::stringstream ss;
    ss << "# HELP crm_total_requests Total HTTP requests\n";
    ss << fmt::format("crm_total_requests {}\n", total_requests_.load());
    ss << "# HELP crm_total_errors Total errors\n";
    ss << fmt::format("crm_total_errors {}\n", total_errors_.load());
    ss << "# HELP crm_db_queries Total DB queries\n";
    ss << fmt::format("crm_db_queries {}\n", total_db_queries_.load());
    ss << "# HELP crm_active_connections Active DB connections\n";
    ss << fmt::format("crm_active_connections {}\n", active_connections_.load());
    
    {
        std::lock_guard lock(counters_mutex_);
        for (const auto& [endpoint, count] : endpoint_counters_) {
            ss << fmt::format("crm_endpoint_requests{{endpoint=\"{}\"}} {}\n", endpoint, count.load());
        }
    }
    
    return ss.str();
}

} // namespace crm::metrics
// Project version: Nexora CRM V2.3
