#include "SDA/Core/Utils/BitSet.h"
#include <gtest/gtest.h>

using namespace utils;

TEST(BitSetTest, testGetSet) {
    BitSet bitSet;
    EXPECT_FALSE(bitSet.get(1000));
    bitSet.set(1000, true);
    EXPECT_TRUE(bitSet.get(1000));
    bitSet.set(1000, false);
    EXPECT_FALSE(bitSet.get(1000));
    EXPECT_EQ(bitSet.size(), 0);
}

TEST(BitSetTest, testOr) {
    BitSet bitSet1;
    bitSet1.set(0, true);
    bitSet1.set(10, true);
    bitSet1.set(100, true);
    BitSet bitSet2;
    bitSet2.set(0, true);
    bitSet2.set(1, true);
    auto result = bitSet1 | bitSet2;
    EXPECT_TRUE(result.get(0));
    EXPECT_TRUE(result.get(1));
    EXPECT_TRUE(result.get(10));
    EXPECT_TRUE(result.get(100));
    EXPECT_FALSE(result.get(2));
    EXPECT_FALSE(result.get(11));
    EXPECT_FALSE(result.get(101));
    EXPECT_EQ(result.size(), 4);
}

TEST(BitSetTest, testAnd) {
    BitSet bitSet1;
    bitSet1.set(0, true);
    bitSet1.set(10, true);
    bitSet1.set(100, true);
    BitSet bitSet2;
    bitSet2.set(0, true);
    bitSet2.set(1, true);
    auto result = bitSet1 & bitSet2;
    EXPECT_TRUE(result.get(0));
    EXPECT_FALSE(result.get(1));
    EXPECT_FALSE(result.get(10));
    EXPECT_FALSE(result.get(100));
    EXPECT_EQ(result.size(), 1);
}

TEST(BitSetTest, testNot) {
    BitSet bitSet;
    bitSet.set(0, true);
    bitSet.set(10, true);
    bitSet.set(100, true);
    auto result = ~bitSet;
    EXPECT_FALSE(result.get(0));
    EXPECT_FALSE(result.get(10));
    EXPECT_FALSE(result.get(100));
    EXPECT_TRUE(result.get(1));
    EXPECT_TRUE(result.get(11));
    EXPECT_TRUE(result.get(1001));
    EXPECT_EQ((~result).size(), 3);
}

TEST(BitSetTest, testEqual) {
    BitSet bitSet1;
    bitSet1.set(0, true);
    bitSet1.set(10, true);
    bitSet1.set(100, true);
    BitSet bitSet2;
    bitSet2.set(0, true);
    bitSet2.set(10, true);
    bitSet2.set(100, true);
    EXPECT_TRUE(bitSet1 == bitSet2);
    bitSet2.set(100, false);
    EXPECT_FALSE(bitSet1 == bitSet2);
}