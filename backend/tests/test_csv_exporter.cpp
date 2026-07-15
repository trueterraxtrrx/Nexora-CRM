#include <gtest/gtest.h>
#include "export/csv_exporter.hpp"
#include <stdexcept>

using crm::export_service::CsvExporter;

TEST(CsvExporterTest, EscapesAndParsesQuotedFields) {
    const std::vector<std::map<std::string, std::string>> rows = {
        {
            {"id", "1"},
            {"name", "Alice, Inc"},
            {"email", "alice@example.com"},
            {"phone", "+100"},
            {"company_name", "A \"Quoted\" Company"},
            {"created_at", "2026-07-13"},
        },
    };

    const auto csv = CsvExporter::clients_to_csv(rows);
    const auto parsed = CsvExporter::parse_csv(csv);

    ASSERT_EQ(parsed.size(), 1u);
    EXPECT_EQ(parsed[0].at("Name"), "Alice, Inc");
    EXPECT_EQ(parsed[0].at("Company"), "A \"Quoted\" Company");
}

TEST(CsvExporterTest, RejectsMalformedRows) {
    EXPECT_THROW(CsvExporter::parse_csv("A,B\n1\n"), std::runtime_error);
    EXPECT_THROW(CsvExporter::parse_csv("A\n\"broken\n"), std::runtime_error);
}

