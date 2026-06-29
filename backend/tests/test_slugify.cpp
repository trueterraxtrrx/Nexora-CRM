#include <gtest/gtest.h>
#include "utils/slugify.hpp"

using namespace crm::utils;

TEST(SlugifyTest, BasicConversion) {
    EXPECT_EQ(slugify("My Company"),  "my-company");
    EXPECT_EQ(slugify("Hello World"), "hello-world");
}

TEST(SlugifyTest, SpecialCharsRemoved) {
    EXPECT_EQ(slugify("Hello, World!"), "hello-world");
    EXPECT_EQ(slugify("A & B Corp."),   "a-b-corp");
}

TEST(SlugifyTest, MultipleSpacesCollapsed) {
    EXPECT_EQ(slugify("A   B   C"), "a-b-c");
}

TEST(SlugifyTest, LeadingTrailingDashRemoved) {
    EXPECT_EQ(slugify("  hello  "), "hello");
    EXPECT_EQ(slugify("!hello!"),   "hello");
}

TEST(SlugifyTest, NumbersPreserved) {
    EXPECT_EQ(slugify("Company 123"), "company-123");
}

TEST(SlugifyTest, MaxLength100) {
    std::string long_name(200, 'a');
    auto result = slugify(long_name);
    EXPECT_LE(result.size(), 100u);
}

TEST(SlugifyTest, EmptyStringHandled) {
    EXPECT_EQ(slugify(""), "");
    EXPECT_EQ(slugify("!!!"), "");
}
// Project version: Nexora CRM V2.3
