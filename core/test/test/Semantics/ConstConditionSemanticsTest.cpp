#include "Test/Core/Semantics/ConstConditionSemanticsFixture.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class ConstConditionSemanticsTest : public ConstConditionSemanticsFixture {};

TEST_F(ConstConditionSemanticsTest, IfElse) {
    auto sourcePCode = "\
        rax:8 = COPY 0:8 \n\
        $1:1 = INT_EQUAL rcx:4, 5:4 \n\
        CBRANCH <labelElse>, $1:1 // if-else condition \n\
        rax:8 = COPY 1:8 // then block \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        rax:8 = COPY 2:8 // else block \n\
        <labelEnd>: \n\
        r10:8 = INT_2COMP rax:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3, far: B5, cond: var3): \n\
            var1[rax]:8 = COPY 0x0:8 \n\
            var2:4 = LOAD rcx \n\
            var3[$U1]:1 = INT_EQUAL var2, 0x5:4 \n\
        Block B3(level: 2, far: B6): \n\
            var4[rax]:8 = COPY 0x1:8 \n\
        Block B5(level: 2, near: B6): \n\
            var5[rax]:8 = COPY 0x2:8 \n\
        Block B6(level: 3): \n\
            var6:8 = REF var4 \n\
            var7:8 = REF var5 \n\
            var8:8 = PHI var6, var7 \n\
            var9[r10]:8 = INT_2COMP var8 \
    ";
    auto expectedConditions = "\
        Block B3: \n\
            var2 != 5 \n\
        Block B5: \n\
            var2 == 5 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpConditions(function, expectedConditions));
}

TEST_F(ConstConditionSemanticsTest, NestedIf) {
    auto sourcePCode = "\
        rax:8 = COPY 0:8 \n\
        $1:1 = INT_NOTEQUAL rcx:4, 5:4 \n\
        CBRANCH <label>, $1:1 \n\
        $2:1 = INT_NOTEQUAL rdx:4, 7:4 \n\
        CBRANCH <label>, $2:1 \n\
        rax:8 = COPY 1:8 \n\
        <label>: \n\
        r10:8 = INT_2COMP rax:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3, far: B6, cond: var3): \n\
            var1[rax]:8 = COPY 0x0:8 \n\
            var2:4 = LOAD rcx \n\
            var3[$U1]:1 = INT_NOTEQUAL var2, 0x5:4 \n\
        Block B3(level: 2, near: B5, far: B6, cond: var5): \n\
            var4:4 = LOAD rdx \n\
            var5[$U2]:1 = INT_NOTEQUAL var4, 0x7:4 \n\
        Block B5(level: 3, near: B6): \n\
            var6[rax]:8 = COPY 0x1:8 \n\
        Block B6(level: 4): \n\
            var7:8 = REF var1 \n\
            var8:8 = REF var6 \n\
            var9:8 = PHI var7, var8 \n\
            var10[r10]:8 = INT_2COMP var9 \
    ";
    auto expectedConditions = "\
        Block B3: \n\
            var2 == 5 \n\
        Block B5: \n\
            var2 == 5 \n\
            var4 == 7 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpConditions(function, expectedConditions));
}

TEST_F(ConstConditionSemanticsTest, Loop) {
    /*
        void func(Object* param1) {
            int i = 0;
            while (i != 3) {
                i++;
            }
        }
    */
    auto sourcePCode = "\
        $1:4 = COPY 0x0:4 \n\
        <loop_cond>: \n\
        $2:1 = INT_EQUAL $1:4, 0x3:4 \n\
        CBRANCH <loop_end>, $2:1 \n\
        $1:4 = INT_ADD $1:4, 0x1:4 \n\
        BRANCH <loop_cond> \n\
        <loop_end>: \n\
        RETURN \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B1): \n\
            var1[$U1]:4 = COPY 0x0:4 \n\
        Block B1(level: 2, near: B3, far: B5, cond: var7): \n\
            var2:4 = REF var1 \n\
            var3:4 = REF var5 \n\
            var6:4 = PHI var2, var3 \n\
            var7[$U2]:1 = INT_EQUAL var6, 0x3:4 \n\
        Block B3(level: 3, far: B1): \n\
            var4:4 = REF var6 \n\
            var5[$U1]:4 = INT_ADD var4, 0x1:4 \n\
        Block B5(level: 3): \n\
            empty \
    ";
    auto expectedConditions = "\
        Block B3: \n\
            var6 != 3 \n\
        Block B5: \n\
            var6 == 3 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpConditions(function, expectedConditions));
}
