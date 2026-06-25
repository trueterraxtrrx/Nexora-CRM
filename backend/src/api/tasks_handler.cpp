#include "api/tasks_handler.hpp"
#include "core/database.hpp"
#include "core/middleware.hpp"
#include "utils/response.hpp"
#include "utils/audit.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <algorithm>
#include <vector>

namespace crm::api {

using namespace crm::core;
using namespace crm::utils;
using json = nlohmann::json;

static json task_to_json(const pqxx::row& r) {
    return {
        {"id",           r["id"].as<int>()},
        {"company_id",   r["company_id"].as<int>()},
        {"title",        r["title"].as<std::string>()},
        {"description",  r["description"].is_null()  ? nullptr : json(r["description"].as<std::string>())},
        {"status",       r["status"].as<std::string>()},
        {"priority",     r["priority"].as<std::string>()},
        {"deadline",     r["deadline"].is_null()     ? nullptr : json(r["deadline"].as<std::string>())},
        {"client_id",    r["client_id"].is_null()    ? nullptr : json(r["client_id"].as<int>())},
        {"assignee_id",  r["assignee_id"].is_null()  ? nullptr : json(r["assignee_id"].as<int>())},
        {"created_at",   r["created_at"].as<std::string>()},
        {"updated_at",   r["updated_at"].as<std::string>()},
        {"completed_at", r["completed_at"].is_null() ? nullptr : json(r["completed_at"].as<std::string>())},
    };
}

void register_tasks_routes(AppType& app) {

    // ── GET /api/v1/tasks ─────────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/tasks").methods(crow::HTTPMethod::Get)
    ([&app](const crow::request& req, crow::response& res) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        int skip  = req.url_params.get("skip")  ? std::stoi(req.url_params.get("skip"))  : 0;
        int limit = req.url_params.get("limit") ? std::stoi(req.url_params.get("limit")) : 50;
        limit = std::min(limit, 200);

        auto status_f   = req.url_params.get("status");
        auto priority_f = req.url_params.get("priority");
        auto overdue_f  = req.url_params.get("overdue");

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                std::string where = "WHERE company_id=" + txn.quote(claims.company_id);

                if (status_f) {
                    where += " AND status=" + txn.quote(std::string(status_f));
                }
                if (priority_f) {
                    where += " AND priority=" + txn.quote(std::string(priority_f));
                }
                if (overdue_f && std::string(overdue_f) == "true") {
                    where += " AND deadline < NOW() AND status NOT IN ('done','cancelled')";
                }

                where += fmt::format(
                    " ORDER BY deadline ASC NULLS LAST, created_at DESC LIMIT {} OFFSET {}",
                    limit, skip
                );

                std::string sql =
                    "SELECT id,company_id,title,description,status,priority,"
                    "deadline,client_id,assignee_id,created_at,updated_at,completed_at "
                    "FROM tasks " + where;

                auto rows = txn.exec(sql);
                json result = json::array();
                for (const auto& row : rows) result.push_back(task_to_json(row));
                return json_ok(result);
            });
        } catch (const std::exception& e) {
            spdlog::error("List tasks: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── POST /api/v1/tasks ────────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/tasks").methods(crow::HTTPMethod::Post)
    ([&app](const crow::request& req, crow::response& res) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        auto body = parse_body(req);
        if (!body || !body->contains("title")) {
            res = json_error(400, "Поле 'title' обязательно");
            return;
        }

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto title       = (*body)["title"].get<std::string>();
                auto description = get_optional<std::string>(*body, "description");
                auto status      = body->contains("status")   ? (*body)["status"].get<std::string>()   : "todo";
                auto priority    = body->contains("priority") ? (*body)["priority"].get<std::string>() : "medium";
                auto deadline    = get_optional<std::string>(*body, "deadline");
                auto client_id   = get_optional<int>(*body, "client_id");
                auto assignee_id = get_optional<int>(*body, "assignee_id");

                auto row = txn.exec_params1(
                    "INSERT INTO tasks(company_id,title,description,status,priority,deadline,client_id,assignee_id) "
                    "VALUES($1,$2,NULLIF($3,''),$4,$5,NULLIF($6,'')::timestamptz,NULLIF($7,'')::int,NULLIF($8,'')::int) "
                    "RETURNING id,company_id,title,description,status,priority,deadline,"
                    "client_id,assignee_id,created_at,updated_at,completed_at",
                    claims.company_id,
                    title,
                    description.value_or(""),
                    status,
                    priority,
                    deadline.value_or(""),
                    client_id ? std::to_string(*client_id) : "",
                    assignee_id ? std::to_string(*assignee_id) : ""
                );

                int new_id = row["id"].as<int>();
                write_audit(txn, claims.company_id, claims.user_id,
                    "create_task", "task", new_id, {{"title", title}});

                return json_created(task_to_json(row));
            });
        } catch (const std::exception& e) {
            spdlog::error("Create task: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── GET /api/v1/tasks/:id ─────────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/tasks/<int>").methods(crow::HTTPMethod::Get)
    ([&app](const crow::request& req, crow::response& res, int task_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;
        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT id,company_id,title,description,status,priority,deadline,"
                    "client_id,assignee_id,created_at,updated_at,completed_at "
                    "FROM tasks WHERE id=$1 AND company_id=$2",
                    task_id, claims.company_id
                );
                if (rows.empty()) return json_error(404, "Задача не найдена");
                return json_ok(task_to_json(rows[0]));
            });
        } catch (const std::exception& e) {
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── PATCH /api/v1/tasks/:id ───────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/tasks/<int>").methods(crow::HTTPMethod::Patch)
    ([&app](const crow::request& req, crow::response& res, int task_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;

        auto body = parse_body(req);
        if (!body) { res = json_error(400, "Invalid JSON"); return; }

        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT status FROM tasks WHERE id=$1 AND company_id=$2",
                    task_id, claims.company_id
                );
                if (rows.empty()) return json_error(404, "Задача не найдена");
                auto old_status = rows[0]["status"].as<std::string>();

                // Если переходим в done — ставим completed_at
                bool going_done = body->contains("status") &&
                    (*body)["status"].get<std::string>() == "done" &&
                    old_status != "done";

                std::vector<std::pair<std::string,std::string>> updates;
                if (body->contains("title"))       updates.push_back({"title",       (*body)["title"].get<std::string>()});
                if (body->contains("status"))      updates.push_back({"status",      (*body)["status"].get<std::string>()});
                if (body->contains("priority"))    updates.push_back({"priority",    (*body)["priority"].get<std::string>()});
                if (body->contains("description")) updates.push_back({"description", (*body)["description"].is_null() ? "" : (*body)["description"].get<std::string>()});
                if (body->contains("deadline"))    updates.push_back({"deadline",    (*body)["deadline"].is_null() ? "" : (*body)["deadline"].get<std::string>()});

                if (updates.empty()) return json_error(400, "Нет полей для обновления");

                std::string set_clause = "updated_at=NOW()";
                if (going_done) set_clause += ",completed_at=NOW()";

                for (auto& [col, val] : updates) {
                    set_clause += fmt::format(",{}={}", col, txn.quote(val));
                }

                std::string sql = "UPDATE tasks SET " + set_clause +
                    fmt::format(" WHERE id={} AND company_id={}"
                    " RETURNING id,company_id,title,description,status,priority,deadline,"
                    "client_id,assignee_id,created_at,updated_at,completed_at",
                    txn.quote(task_id), txn.quote(claims.company_id));

                auto updated = txn.exec1(sql);
                write_audit(txn, claims.company_id, claims.user_id,
                    "update_task", "task", task_id, *body);

                return json_ok(task_to_json(updated));
            });
        } catch (const std::exception& e) {
            spdlog::error("Update task: {}", e.what());
            res = json_error(500, "Внутренняя ошибка");
        }
    });

    // ── DELETE /api/v1/tasks/:id ──────────────────────────────────────────
    CROW_ROUTE(app, "/api/v1/tasks/<int>").methods(crow::HTTPMethod::Delete)
    ([&app](const crow::request& req, crow::response& res, int task_id) {
        auto& ctx = app.get_context<AuthMiddleware>(req);
        if (!require_auth(req, res, ctx)) return;
        auto& claims = *ctx.claims;
        try {
            res = get_db().with_transaction<crow::response>([&](pqxx::work& txn) {
                auto rows = txn.exec_params(
                    "SELECT title FROM tasks WHERE id=$1 AND company_id=$2",
                    task_id, claims.company_id
                );
                if (rows.empty()) return json_error(404, "Задача не найдена");
                auto title = rows[0]["title"].as<std::string>();

                txn.exec_params("DELETE FROM tasks WHERE id=$1", task_id);
                write_audit(txn, claims.company_id, claims.user_id,
                    "delete_task", "task", task_id, {{"title", title}});
                return json_no_content();
            });
        } catch (const std::exception& e) {
            res = json_error(500, "Внутренняя ошибка");
        }
    });
}

} // namespace crm::api
