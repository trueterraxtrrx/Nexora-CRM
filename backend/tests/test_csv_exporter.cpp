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

TEST(CsvExporterTest, ValidatesRequiredImportHeaders) {
    const auto parsed = CsvExporter::parse_csv("Name,Email\nAlice,alice@example.com\n");

    EXPECT_NO_THROW(CsvExporter::require_headers(parsed, {"Name", "Email"}));
    EXPECT_THROW(CsvExporter::require_headers(parsed, {"Name", "Company"}), std::runtime_error);
}

TEST(CsvExporterTest, SumsFinanceImportAmounts) {
    const auto parsed = CsvExporter::parse_csv("Amount,Category\n125.50,hosting\n74.50,support\n");

    EXPECT_DOUBLE_EQ(CsvExporter::sum_numeric_column(parsed, "Amount"), 200.0);
}

TEST(CsvExporterTest, CountsRowsMissingImportValues) {
    const auto parsed = CsvExporter::parse_csv("Name,Email\nAlice,alice@example.com\nBob,\n");

    EXPECT_EQ(CsvExporter::count_rows_missing_value(parsed, "Email"), 1u);
}

