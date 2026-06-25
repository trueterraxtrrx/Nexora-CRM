#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>
#include <pqxx/pqxx>

namespace crm::core {

class ConnectionPool {
public:
    using ConnectionPtr = std::unique_ptr<pqxx::connection, std::function<void(pqxx::connection*)>>;

    ConnectionPool(std::string conn_string, int pool_size)
        : conn_string_(std::move(conn_string)), pool_size_(pool_size > 0 ? pool_size : 1) {
        for (int i = 0; i < pool_size_; ++i) {
            pool_.push_back(std::make_unique<pqxx::connection>(conn_string_));
        }
    }

    ConnectionPtr acquire() {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [&] { return !pool_.empty(); });
        auto conn = std::move(pool_.back());
        pool_.pop_back();
        return ConnectionPtr(conn.release(), [this](pqxx::connection* raw) {
            std::unique_ptr<pqxx::connection> returned(raw);
            {
                std::lock_guard lock(mutex_);
                if (returned && returned->is_open()) {
                    pool_.push_back(std::move(returned));
                }
            }
            cv_.notify_one();
        });
    }

    template <typename T, typename Fn>
    T with_transaction(Fn&& fn) {
        auto conn = acquire();
        pqxx::work txn(*conn);
        T result = fn(txn);
        txn.commit();
        return result;
    }

private:
    std::string conn_string_;
    int pool_size_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::unique_ptr<pqxx::connection>> pool_;
};

void init_db(const std::string& conn_string, int pool_size);
ConnectionPool& get_db();
void run_migrations(pqxx::connection& conn);

} // namespace crm::core
