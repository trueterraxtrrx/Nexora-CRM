#pragma once

#include <map>
#include <string>
#include <vector>

namespace crm::export_service {

class CsvExporter {
public:
    static std::string clients_to_csv(const std::vector<std::map<std::string, std::string>>& data);
    static std::string tasks_to_csv(const std::vector<std::map<std::string, std::string>>& data);
    static std::string finance_to_csv(const std::vector<std::map<std::string, std::string>>& data);
    static std::vector<std::map<std::string, std::string>> parse_csv(const std::string& csv);

private:
    static std::string escape_csv(const std::string& field);
    static std::vector<std::string> parse_csv_line(const std::string& line);
};

} // namespace crm::export_service
// Project version: Nexora CRM V2.7




