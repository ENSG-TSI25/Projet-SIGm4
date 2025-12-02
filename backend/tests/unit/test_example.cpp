#include <gtest/gtest.h>

// Basic Test
TEST(ExampleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

TEST(ExampleTest, StringTest) {
    std::string str = "Hello";
    EXPECT_EQ(str, "Hello");
    EXPECT_NE(str, "World");
}

