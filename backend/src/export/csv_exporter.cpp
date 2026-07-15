#include "export/csv_exporter.hpp"
#include <stdexcept>
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

std::vector<std::string> CsvExporter::parse_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quoted && c == '"' && i + 1 < line.size() && line[i + 1] == '"') {
            current.push_back('"');
            ++i;
        } else if (c == '"') {
            quoted = !quoted;
        } else if (c == ',' && !quoted) {
            fields.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    if (quoted) {
        throw std::runtime_error("unterminated CSV quote");
    }
    fields.push_back(current);
    return fields;
}

std::vector<std::map<std::string, std::string>> CsvExporter::parse_csv(const std::string& csv) {
    std::stringstream input(csv);
    std::string line;
    if (!std::getline(input, line)) {
        return {};
    }
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    const auto headers = parse_csv_line(line);
    std::vector<std::map<std::string, std::string>> rows;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        const auto fields = parse_csv_line(line);
        if (fields.size() != headers.size()) {
            throw std::runtime_error("CSV row width mismatch");
        }
        std::map<std::string, std::string> row;
        for (std::size_t i = 0; i < headers.size(); ++i) {
            row[headers[i]] = fields[i];
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

void CsvExporter::require_headers(const std::vector<std::map<std::string, std::string>>& rows, const std::vector<std::string>& headers) {
    for (const auto& row : rows) {
        for (const auto& header : headers) {
            if (row.find(header) == row.end()) {
                throw std::runtime_error("missing CSV header: " + header);
            }
        }
    }
}

} // namespace crm::export_service
// Project version: Nexora CRM V2.7




