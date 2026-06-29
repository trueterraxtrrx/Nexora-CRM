#include "export/csv_exporter.hpp"
#include <sstream>

namespace crm::export_service {

std::string CsvExporter::escape_csv(const std::string& field) {
    if (field.find(',') == std::string::npos && field.find('"') == std::string::npos) {
        return field;
    }
    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') escaped += "\"\"";
        else escaped += c;
    }
    escaped += "\"";
    return escaped;
}

std::string CsvExporter::clients_to_csv(const std::vector<std::map<std::string, std::string>>& data) {
    std::stringstream ss;
    ss << "ID,Name,Email,Phone,Company,Created\n";
    for (const auto& row : data) {
        ss << escape_csv(row.at("id")) << ","
           << escape_csv(row.at("name")) << ","
           << escape_csv(row.at("email")) << ","
           << escape_csv(row.at("phone")) << ","
           << escape_csv(row.at("company_name")) << ","
           << escape_csv(row.at("created_at")) << "\n";
    }
    return ss.str();
}

std::string CsvExporter::tasks_to_csv(const std::vector<std::map<std::string, std::string>>& data) {
    std::stringstream ss;
    ss << "ID,Title,Status,Priority,Deadline,Assigned,Created\n";
    for (const auto& row : data) {
        ss << escape_csv(row.at("id")) << ","
           << escape_csv(row.at("title")) << ","
           << escape_csv(row.at("status")) << ","
           << escape_csv(row.at("priority")) << ","
           << escape_csv(row.at("deadline")) << ","
           << escape_csv(row.at("assignee")) << ","
           << escape_csv(row.at("created_at")) << "\n";
    }
    return ss.str();
}

std::string CsvExporter::finance_to_csv(const std::vector<std::map<std::string, std::string>>& data) {
    std::stringstream ss;
    ss << "ID,Type,Amount,Currency,Category,Description,Date\n";
    for (const auto& row : data) {
        ss << escape_csv(row.at("id")) << ","
           << escape_csv(row.at("type")) << ","
           << escape_csv(row.at("amount")) << ","
           << escape_csv(row.at("currency")) << ","
           << escape_csv(row.at("category")) << ","
           << escape_csv(row.at("description")) << ","
           << escape_csv(row.at("date")) << "\n";
    }
    return ss.str();
}

} // namespace crm::export_service
// Project version: Nexora CRM V2.3
