#include "SDA/Core/Utils/BitMask.h"
#include <gtest/gtest.h>

using namespace utils;

TEST(BitMaskTest, init1) {
    BitMask bitMask(1, 0);
    EXPECT_EQ(bitMask, 0xFF);
}

TEST(BitMaskTest, init2) {
    BitMask bitMask(8, 0);
    EXPECT_EQ(bitMask, 0xFFFFFFFFFFFFFFFF);
}

TEST(BitMaskTest, init3) {
    BitMask bitMask(1, 1);
    EXPECT_EQ(bitMask, 0xFF00);
}

TEST(BitMaskTest, getSize) {
    BitMask bitMask(0xFF00);
    EXPECT_EQ(bitMask.getSize(), 1);
}

TEST(BitMaskTest, getOffset) {
    BitMask bitMask(0xFF00);
    EXPECT_EQ(bitMask.getOffset(), 8);
}
