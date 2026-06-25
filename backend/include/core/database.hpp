#pragma once
#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace crm::core {

// ─── Connection Pool ─────────────────────────────────────────────────────────
// Thread-safe пул соединений к PostgreSQL.
// Каждый поток берёт соединение из пула, использует, возвращает.

class ConnectionPool {
public:
    explicit ConnectionPool(const std::string& conn_string, int pool_size = 10)
        : conn_string_(conn_string)
    {
        for (int i = 0; i < pool_size; ++i) {
            pool_.emplace(std::make_unique<pqxx::connection>(conn_string_));
            spdlog::debug("DB connection {} created", i + 1);
        }
        spdlog::info("Connection pool initialized: {} connections", pool_size);
    }

    // RAII-обёртка — гарантирует возврат соединения в пул
    class ScopedConnection {
    public:
        ScopedConnection(ConnectionPool& pool, std::unique_ptr<pqxx::connection> conn)
            : pool_(pool), conn_(std::move(conn)) {}

        ~ScopedConnection() {
            pool_.return_connection(std::move(conn_));
        }

        pqxx::connection& get() { return *conn_; }
        pqxx::connection* operator->() { return conn_.get(); }

        // Некопируемый
        ScopedConnection(const ScopedConnection&) = delete;
        ScopedConnection& operator=(const ScopedConnection&) = delete;

    private:
        ConnectionPool& pool_;
        std::unique_ptr<pqxx::connection> conn_;
    };

    [[nodiscard]] ScopedConnection acquire() {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return !pool_.empty(); });

        auto conn = std::move(pool_.front());
        pool_.pop();

        // Reconnect если соединение упало
        if (!conn->is_open()) {
            spdlog::warn("DB connection lost, reconnecting...");
            conn = std::make_unique<pqxx::connection>(conn_string_);
        }

        return ScopedConnection(*this, std::move(conn));
    }

    // Транзакция-хелпер: берёт соединение, выполняет лямбду, возвращает
    template<typename Func>
    auto with_transaction(Func&& func) -> decltype(func(std::declval<pqxx::work&>())) {
        auto conn = acquire();
        pqxx::work txn(conn.get());
        try {
            auto result = func(txn);
            txn.commit();
            return result;
        } catch (...) {
            txn.abort();
            throw;
        }
    }

    // Версия без возврата значения
    void execute(std::function<void(pqxx::work&)> func) {
        auto conn = acquire();
        pqxx::work txn(conn.get());
        try {
            func(txn);
            txn.commit();
        } catch (...) {
            txn.abort();
            throw;
        }
    }

private:
    void return_connection(std::unique_ptr<pqxx::connection> conn) {
        std::lock_guard lock(mutex_);
        pool_.push(std::move(conn));
        cv_.notify_one();
    }

    std::string conn_string_;
    std::queue<std::unique_ptr<pqxx::connection>> pool_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

// Global singleton
ConnectionPool& get_db();
void init_db(const std::string& conn_string, int pool_size);

// Запуск SQL миграций при старте
void run_migrations(pqxx::connection& conn);

} // namespace crm::core
