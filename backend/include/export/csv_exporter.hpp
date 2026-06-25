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

private:
    static std::string escape_csv(const std::string& field);
};

} // namespace crm::export_service
