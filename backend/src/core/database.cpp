#include "core/database.hpp"
#include <spdlog/spdlog.h>
#include <memory>

namespace crm::core {

static std::unique_ptr<ConnectionPool> g_pool;

ConnectionPool& get_db() {
    if (!g_pool) throw std::runtime_error("Database not initialized");
    return *g_pool;
}

void init_db(const std::string& conn_string, int pool_size) {
    g_pool = std::make_unique<ConnectionPool>(conn_string, pool_size);
}

// ─── SQL Migrations ──────────────────────────────────────────────────────────
// В проде используй отдельный инструмент миграций (flyway / sqitch).
// Здесь — встроенные CREATE TABLE IF NOT EXISTS для простоты.

void run_migrations(pqxx::connection& conn) {
    spdlog::info("Running DB migrations...");

    pqxx::work txn(conn);

    // Enums
    txn.exec(R"SQL(
        DO $$ BEGIN
            CREATE TYPE user_role AS ENUM ('admin','manager','user');
        EXCEPTION WHEN duplicate_object THEN NULL; END $$;

        DO $$ BEGIN
            CREATE TYPE subscription_plan AS ENUM ('free','pro','enterprise');
        EXCEPTION WHEN duplicate_object THEN NULL; END $$;

        DO $$ BEGIN
            CREATE TYPE task_status AS ENUM ('todo','in_progress','done','cancelled');
        EXCEPTION WHEN duplicate_object THEN NULL; END $$;

        DO $$ BEGIN
            CREATE TYPE task_priority AS ENUM ('low','medium','high','urgent');
        EXCEPTION WHEN duplicate_object THEN NULL; END $$;

        DO $$ BEGIN
            CREATE TYPE finance_type AS ENUM ('income','expense');
        EXCEPTION WHEN duplicate_object THEN NULL; END $$;
    )SQL");

    // Companies (tenants)
    txn.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS companies (
            id          SERIAL PRIMARY KEY,
            name        VARCHAR(255) NOT NULL,
            slug        VARCHAR(100) NOT NULL UNIQUE,
            plan        subscription_plan NOT NULL DEFAULT 'free',
            is_active   BOOLEAN NOT NULL DEFAULT TRUE,
            max_users   INT NOT NULL DEFAULT 3,
            max_clients INT NOT NULL DEFAULT 50,
            created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW()
        );
        CREATE INDEX IF NOT EXISTS idx_companies_slug ON companies(slug);
    )SQL");

    // Users
    txn.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS users (
            id                SERIAL PRIMARY KEY,
            company_id        INT NOT NULL REFERENCES companies(id) ON DELETE CASCADE,
            email             VARCHAR(255) NOT NULL UNIQUE,
            hashed_password   VARCHAR(255) NOT NULL,
            full_name         VARCHAR(255),
            role              user_role NOT NULL DEFAULT 'user',
            is_active         BOOLEAN NOT NULL DEFAULT TRUE,
            avatar_url        VARCHAR(500),
            notify_email      BOOLEAN NOT NULL DEFAULT TRUE,
            telegram_chat_id  VARCHAR(50),
            created_at        TIMESTAMPTZ NOT NULL DEFAULT NOW(),
            last_login        TIMESTAMPTZ
        );
        CREATE INDEX IF NOT EXISTS idx_users_company ON users(company_id);
        CREATE INDEX IF NOT EXISTS idx_users_email   ON users(email);
    )SQL");

    // Clients
    txn.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS clients (
            id           SERIAL PRIMARY KEY,
            company_id   INT NOT NULL REFERENCES companies(id) ON DELETE CASCADE,
            name         VARCHAR(255) NOT NULL,
            email        VARCHAR(255),
            phone        VARCHAR(50),
            company_name VARCHAR(255),
            notes        TEXT,
            tags         VARCHAR(500),
            is_active    BOOLEAN NOT NULL DEFAULT TRUE,
            created_at   TIMESTAMPTZ NOT NULL DEFAULT NOW(),
            updated_at   TIMESTAMPTZ NOT NULL DEFAULT NOW()
        );
        CREATE INDEX IF NOT EXISTS idx_clients_company ON clients(company_id);
        CREATE INDEX IF NOT EXISTS idx_clients_email   ON clients(email);
    )SQL");

    // Client interactions
    txn.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS client_interactions (
            id         SERIAL PRIMARY KEY,
            client_id  INT NOT NULL REFERENCES clients(id) ON DELETE CASCADE,
            user_id    INT NOT NULL REFERENCES users(id),
            type       VARCHAR(50) NOT NULL,
            content    TEXT NOT NULL,
            created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
        );
        CREATE INDEX IF NOT EXISTS idx_interactions_client ON client_interactions(client_id);
    )SQL");

    // Tasks
    txn.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS tasks (
            id           SERIAL PRIMARY KEY,
            company_id   INT NOT NULL REFERENCES companies(id) ON DELETE CASCADE,
            client_id    INT REFERENCES clients(id) ON DELETE SET NULL,
            assignee_id  INT REFERENCES users(id) ON DELETE SET NULL,
            title        VARCHAR(500) NOT NULL,
            description  TEXT,
            status       task_status NOT NULL DEFAULT 'todo',
            priority     task_priority NOT NULL DEFAULT 'medium',
            deadline     TIMESTAMPTZ,
            created_at   TIMESTAMPTZ NOT NULL DEFAULT NOW(),
            updated_at   TIMESTAMPTZ NOT NULL DEFAULT NOW(),
            completed_at TIMESTAMPTZ
        );
        CREATE INDEX IF NOT EXISTS idx_tasks_company  ON tasks(company_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_status   ON tasks(status);
        CREATE INDEX IF NOT EXISTS idx_tasks_deadline ON tasks(deadline);
    )SQL");

    // Finance
    txn.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS finances (
            id          SERIAL PRIMARY KEY,
            company_id  INT NOT NULL REFERENCES companies(id) ON DELETE CASCADE,
            client_id   INT REFERENCES clients(id) ON DELETE SET NULL,
            type        finance_type NOT NULL,
            amount      NUMERIC(12,2) NOT NULL,
            currency    VARCHAR(3) NOT NULL DEFAULT 'RUB',
            description VARCHAR(500),
            category    VARCHAR(100),
            date        TIMESTAMPTZ NOT NULL DEFAULT NOW(),
            created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW()
        );
        CREATE INDEX IF NOT EXISTS idx_finances_company ON finances(company_id);
        CREATE INDEX IF NOT EXISTS idx_finances_date    ON finances(date);
        CREATE INDEX IF NOT EXISTS idx_finances_type    ON finances(type);
    )SQL");

    // Audit log
    txn.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS audit_logs (
            id         SERIAL PRIMARY KEY,
            company_id INT NOT NULL REFERENCES companies(id) ON DELETE CASCADE,
            user_id    INT REFERENCES users(id) ON DELETE SET NULL,
            action     VARCHAR(100) NOT NULL,
            entity     VARCHAR(100),
            entity_id  INT,
            details    JSONB,
            ip_address VARCHAR(45),
            created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
        );
        CREATE INDEX IF NOT EXISTS idx_audit_company ON audit_logs(company_id);
        CREATE INDEX IF NOT EXISTS idx_audit_created ON audit_logs(created_at DESC);
    )SQL");

    txn.commit();
    spdlog::info("Migrations completed successfully");
}

} // namespace crm::core
