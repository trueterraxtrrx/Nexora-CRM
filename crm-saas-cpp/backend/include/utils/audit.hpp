#pragma once
#include <pqxx/pqxx>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace crm::utils {

void write_audit(
    pqxx::work& txn,
    int company_id,
    std::optional<int> user_id,
    const std::string& action,
    const std::string& entity,
    int entity_id,
    const nlohmann::json& details = {}
);

} // namespace crm::utils
