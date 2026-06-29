#include <gtest/gtest.h>
#include "core/password.hpp"

using namespace crm::core;

TEST(PasswordTest, HashIsNotPlaintext) {
    auto hash = hash_password("mysecretpassword");
    EXPECT_NE(hash, "mysecretpassword");
    EXPECT_GT(hash.size(), 20u);
}

TEST(PasswordTest, VerifyCorrectPassword) {
    auto hash = hash_password("correct_password_123");
    EXPECT_TRUE(verify_password("correct_password_123", hash));
}

TEST(PasswordTest, RejectWrongPassword) {
    auto hash = hash_password("correct_password_123");
    EXPECT_FALSE(verify_password("wrong_password", hash));
}

TEST(PasswordTest, TwoHashesDiffer) {
    // Bcrypt использует случайную соль — одинаковые пароли дают разные хэши
    auto h1 = hash_password("same_password");
    auto h2 = hash_password("same_password");
    EXPECT_NE(h1, h2);

    // Но оба валидны
    EXPECT_TRUE(verify_password("same_password", h1));
    EXPECT_TRUE(verify_password("same_password", h2));
}

TEST(PasswordTest, EmptyPasswordHandled) {
    auto hash = hash_password("");
    EXPECT_FALSE(hash.empty());
    EXPECT_TRUE(verify_password("", hash));
    EXPECT_FALSE(verify_password("not_empty", hash));
}
// Project version: Nexora CRM V2.3
