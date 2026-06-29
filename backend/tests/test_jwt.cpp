#include <gtest/gtest.h>
#include "core/jwt.hpp"

using namespace crm::core;

class JwtTest : public ::testing::Test {
protected:
    JwtService jwt{"test_secret_key_for_unit_tests_32x", 24};
};

TEST_F(JwtTest, CreateAndVerify) {
    auto token = jwt.create_token(42, 7, "admin");
    EXPECT_FALSE(token.empty());

    auto claims = jwt.verify_token(token);
    ASSERT_TRUE(claims.has_value());
    EXPECT_EQ(claims->user_id,    42);
    EXPECT_EQ(claims->company_id, 7);
    EXPECT_EQ(claims->role,       "admin");
}

TEST_F(JwtTest, InvalidTokenReturnsNullopt) {
    auto result = jwt.verify_token("not.a.valid.token");
    EXPECT_FALSE(result.has_value());
}

TEST_F(JwtTest, TamperedTokenRejected) {
    auto token = jwt.create_token(1, 1, "user");
    // Меняем последний символ подписи
    auto tampered = token;
    tampered.back() = (tampered.back() == 'A') ? 'B' : 'A';

    auto result = jwt.verify_token(tampered);
    EXPECT_FALSE(result.has_value());
}

TEST_F(JwtTest, WrongSecretRejected) {
    auto token = jwt.create_token(1, 1, "user");
    JwtService other_jwt{"completely_different_secret_key!", 24};
    auto result = other_jwt.verify_token(token);
    EXPECT_FALSE(result.has_value());
}

TEST_F(JwtTest, DifferentRolesEncoded) {
    for (const auto& role : {"admin", "manager", "user"}) {
        auto token = jwt.create_token(1, 1, role);
        auto claims = jwt.verify_token(token);
        ASSERT_TRUE(claims.has_value());
        EXPECT_EQ(claims->role, role);
    }
}
// Project version: Nexora CRM V2.3
