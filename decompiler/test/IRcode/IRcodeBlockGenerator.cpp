#include <gtest/gtest.h>

TEST(BlockGenerator, BasicAssertions1) {
  EXPECT_EQ(7 * 6, 42);
}

TEST(BlockGenerator, BasicAssertions2) {
  EXPECT_STREQ("hello", "world");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}