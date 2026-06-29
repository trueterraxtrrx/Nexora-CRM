#include "utils/audit.hpp"
#include <spdlog/spdlog.h>

namespace crm::utils {

void write_audit(
    pqxx::work& txn,
    int company_id,
    std::optional<int> user_id,
    const std::string& action,
    const std::string& entity,
    int entity_id,
    const nlohmann::json& details
) {
    try {
        if (user_id.has_value()) {
            txn.exec_params(
                "INSERT INTO audit_logs(company_id,user_id,action,entity,entity_id,details) "
                "VALUES($1,$2,$3,$4,$5,$6::jsonb)",
                company_id, *user_id, action, entity, entity_id, details.dump()
            );
        } else {
            txn.exec_params(
                "INSERT INTO audit_logs(company_id,action,entity,entity_id,details) "
                "VALUES($1,$2,$3,$4,$5::jsonb)",
                company_id, action, entity, entity_id, details.dump()
            );
        }
    } catch (const std::exception& e) {
        // Ошибка аудита не должна ломать основной запрос
        spdlog::error("Audit log failed: {}", e.what());
    }
}

} // namespace crm::utils
// Project version: Nexora CRM V2.3
