#include <gtest/gtest.h>

TEST(ModuleTest, Test1) {
    // Arrange
    int expected = 42;
    
    // Act
    int result = 42;
    
    // Assert
    EXPECT_EQ(result, expected);
}

TEST(ModuleTest, Test2) {
    EXPECT_TRUE(true);
}