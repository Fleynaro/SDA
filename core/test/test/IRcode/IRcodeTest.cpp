#include "Test/Core/IRcode/IRcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class IRcodeTest : public IRcodeFixture
{
protected:
    pcode::Graph graph;
    ircode::Program program = ircode::Program(&graph);

    ::testing::AssertionResult cmp(ircode::Function* function, const std::string& expectedCode) const {
        std::stringstream ss;
        printIRcode(function, ss, 2);
        return Compare(ss.str(), expectedCode);
    }
};

TEST_F(IRcodeTest, Simplest) {
    // Graph: https://photos.app.goo.gl/qZr7FEV3H7bY9d9Q7
    auto sourcePCode = "\
        rax:8 = COPY 1:8 \n\
        rax:8 = INT_ADD rax:8, 1:8 \
    ";
    auto expectedIRode = "\
        Block B0(level: 1): \n\
            var1[rax]:8 = COPY 0x1:8 \n\
            var2[rax]:8 = INT_ADD var1, 0x1:8 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}

TEST_F(IRcodeTest, SimpleConcat) {
    // Graph: https://photos.app.goo.gl/aHmMvgUYDTZnQQTm7
    auto sourcePCode = "\
        rax:8 = COPY 0x2100:8 \n\
        BRANCH <label> \n\
        <label>: \n\
        rax:1 = COPY 0x34:1 \n\
        r10:2 = INT_2COMP rax:2 \n\
    ";
    auto expectedIRode = "\
        Block B0(level: 1, far: B2): \n\
            var1[rax]:8 = COPY 0x2100:8 \n\
        Block B2(level: 2): \n\
            var2[rax]:1 = COPY 0x34:1 \n\
            var3[rax]:2 = EXTRACT var1, 0 \n\
            var4:2 = CONCAT var3, var2, 0 \n\
            var5[r10]:2 = INT_2COMP var4 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}

TEST_F(IRcodeTest, IfElseCondition) {
    // Graph: https://photos.app.goo.gl/c3siEJg2nAdYckfZ7
    auto sourcePCode = "\
        rax:8 = COPY 0:8 \n\
        CBRANCH <labelElse>, 0:1 // if-else condition \n\
        rax:8 = COPY 1:8 // then block \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        NOP // else block \n\
        <labelEnd>: \n\
        r10:8 = INT_2COMP rax:8 \
    ";
    auto expectedIRode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            var1[rax]:8 = COPY 0x0:8 \n\
        Block B2(level: 2, far: B5): \n\
            var2[rax]:8 = COPY 0x1:8 \n\
        Block B4(level: 2, near: B5): \n\
            empty \n\
        Block B5(level: 3): \n\
            var3:8 = PHI var2, var1 \n\
            var4[r10]:8 = INT_2COMP var3 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}

TEST_F(IRcodeTest, IfElseConditionMem) {
    // Graph: https://photos.app.goo.gl/JoBQWzpEHzBiyQcX6
    auto sourcePCode = "\
        rax:8 = INT_ADD rsp:8, 0x10:8 \n\
        STORE rax:8, 0x0:8 \n\
        CBRANCH <labelElse>, 0:1 // if-else condition \n\
        STORE rax:8, 1:8 // then block \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        NOP // else block \n\
        <labelEnd>: \n\
        r10:8 = LOAD rax:8, 8:8 \
    ";
    auto expectedIRode = "\
        Block B0(level: 1, near: B3, far: B5): \n\
            var1:8 = LOAD rsp \n\
            var2[rax]:8 = INT_ADD var1, 0x10:8 \n\
            var3[var1 + 16]:8 = COPY 0x0:8 \n\
        Block B3(level: 2, far: B6): \n\
            var4[var1 + 16]:8 = COPY 0x1:8 \n\
        Block B5(level: 2, near: B6): \n\
            empty \n\
        Block B6(level: 3): \n\
            var5:8 = PHI var4, var3 \n\
            var6[r10]:8 = COPY var5 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}

TEST_F(IRcodeTest, IfElseConditionAlAh) {
    // Graph: https://photos.app.goo.gl/hrLtpD82F22gxLfE8
    auto sourcePCode = "\
        NOP \n\
        CBRANCH <labelElse>, 0:1 // if-else condition \n\
        rax:1:0 = COPY 1:1 // then block \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        rax:1:1 = COPY 2:1 // else block \n\
        <labelEnd>: \n\
        r10:2 = INT_2COMP rax:2 \
    ";
    auto expectedIRode = "\
        Block B0(level: 1, near: B2, far: B4): \n\
            empty \n\
        Block B2(level: 2, far: B5): \n\
            var1[rax]:1 = COPY 0x1:1 \n\
        Block B4(level: 2, near: B5): \n\
            var2[rax + 1]:1 = COPY 0x2:1 \n\
        Block B5(level: 3): \n\
            var3:2 = LOAD rax \n\
            var4:2 = CONCAT var3, var1, 0 \n\
            var5:2 = CONCAT var3, var2, 1 \n\
            var6:2 = PHI var4, var5 \n\
            var7[r10]:2 = INT_2COMP var6 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}

TEST_F(IRcodeTest, IfElseConditionXmm) {
    // Graph: https://photos.app.goo.gl/MyxDwPBjKYKYK13P9
    auto sourcePCode = "\
        xmm0:Da = COPY 1:4 \n\
        xmm0:Db = COPY 2:4 \n\
        xmm0:Dc = COPY 3:4 \n\
        xmm0:Dd = COPY 4:4 \n\
        CBRANCH <labelElse>, 0:1 // if-else condition \n\
        xmm0:Da = INT_ADD xmm0:Da, 1:4 // then block \n\
        xmm0:Db = INT_ADD xmm0:Db, 2:4 \n\
        xmm0:Dc = INT_ADD xmm0:Dc, 3:4 \n\
        xmm0:Dd = INT_ADD xmm0:Dd, 4:4 \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        xmm0:Da = INT_SUB xmm0:Da, 1:4 // else block \n\
        xmm0:Db = INT_SUB xmm0:Db, 2:4 \n\
        xmm0:Dc = INT_SUB xmm0:Dc, 3:4 \n\
        xmm0:Dd = INT_SUB xmm0:Dd, 4:4 \n\
        <labelEnd>: \n\
        rax:8 = INT_ADD rsp:8, 0:8 \n\
        STORE rax:8, xmm0:Da \n\
        rax:8 = INT_ADD rsp:8, 4:8 \n\
        STORE rax:8, xmm0:Db \n\
        rax:8 = INT_ADD rsp:8, 8:8 \n\
        STORE rax:8, xmm0:Dc \n\
        rax:8 = INT_ADD rsp:8, 12:8 \n\
        STORE rax:8, xmm0:Dd \
    ";
    auto expectedIRode = "\
        Block B0(level: 1, near: B5, far: Ba): \n\
            var1[xmm0]:4 = COPY 0x1:4 \n\
            var2[xmm0 + 4]:4 = COPY 0x2:4 \n\
            var3[xmm0 + 8]:4 = COPY 0x3:4 \n\
            var4[xmm0 + 12]:4 = COPY 0x4:4 \n\
        Block B5(level: 2, far: Be): \n\
            var5[xmm0]:4 = INT_ADD var1, 0x1:4 \n\
            var6[xmm0 + 4]:4 = INT_ADD var2, 0x2:4 \n\
            var7[xmm0 + 8]:4 = INT_ADD var3, 0x3:4 \n\
            var8[xmm0 + 12]:4 = INT_ADD var4, 0x4:4 \n\
        Block Ba(level: 2, near: Be): \n\
            var9[xmm0]:4 = INT_SUB var1, 0x1:4 \n\
            var10[xmm0 + 4]:4 = INT_SUB var2, 0x2:4 \n\
            var11[xmm0 + 8]:4 = INT_SUB var3, 0x3:4 \n\
            var12[xmm0 + 12]:4 = INT_SUB var4, 0x4:4 \n\
        Block Be(level: 3): \n\
            var13:8 = LOAD rsp \n\
            var14[rax]:8 = INT_ADD var13, 0x0:8 \n\
            var15:4 = PHI var5, var9 \n\
            var16[var13]:4 = COPY var15 \n\
            var17[rax]:8 = INT_ADD var13, 0x4:8 \n\
            var18:4 = PHI var6, var10 \n\
            var19[var13 + 4]:4 = COPY var18 \n\
            var20[rax]:8 = INT_ADD var13, 0x8:8 \n\
            var21:4 = PHI var7, var11 \n\
            var22[var13 + 8]:4 = COPY var21 \n\
            var23[rax]:8 = INT_ADD var13, 0xc:8 \n\
            var24:4 = PHI var8, var12 \n\
            var25[var13 + 12]:4 = COPY var24 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}

TEST_F(IRcodeTest, MultipleParentBlocksNoPhi) {
    // Graph: https://photos.app.goo.gl/UsYTBTCBcx4hm5jT8
    auto sourcePCode = "\
        rax:8 = COPY 1:8 \n\
        CBRANCH <labelElse>, 0:1 \n\
        NOP \n\
        CBRANCH <labelElse2>, 0:1 \n\
        NOP \n\
        BRANCH <labelEnd> \n\
        <labelElse2>: \n\
        NOP \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        NOP \n\
        <labelEnd>: \n\
        r10:8 = INT_2COMP rax:8 \
    ";
    auto expectedIRode = "\
        Block B0(level: 1, near: B2, far: B8): \n\
            var1[rax]:8 = COPY 0x1:8 \n\
        Block B2(level: 2, near: B4, far: B6): \n\
            empty \n\
        Block B8(level: 2, near: B9): \n\
            empty \n\
        Block B4(level: 3, far: B9): \n\
            empty \n\
        Block B6(level: 3, far: B9): \n\
            empty \n\
        Block B9(level: 4): \n\
            var2[r10]:8 = INT_2COMP var1 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}

TEST_F(IRcodeTest, MultipleParentBlocksDoublePhi) {
    // Graph: https://photos.app.goo.gl/5BbLh6Ufc2Kyu7Zg7
    auto sourcePCode = "\
        NOP \n\
        CBRANCH <labelElse>, 0:1 \n\
        rax:8 = COPY 0:8 \n\
        CBRANCH <labelElse2>, 0:1 \n\
        rax:8 = COPY 1:8 \n\
        BRANCH <labelEnd> \n\
        <labelElse2>: \n\
        NOP \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        rax:8 = COPY 2:8 \n\
        <labelEnd>: \n\
        r10:8 = INT_2COMP rax:8 \
    ";
    auto expectedIRode = "\
        Block B0(level: 1, near: B2, far: B8): \n\
            empty \n\
        Block B2(level: 2, near: B4, far: B6): \n\
            var1[rax]:8 = COPY 0x0:8 \n\
        Block B8(level: 2, near: B9): \n\
            var2[rax]:8 = COPY 0x2:8 \n\
        Block B4(level: 3, far: B9): \n\
            var3[rax]:8 = COPY 0x1:8 \n\
        Block B6(level: 3, far: B9): \n\
            empty \n\
        Block B9(level: 4): \n\
            var4:8 = PHI var3, var2 \n\
            var5:8 = PHI var4, var1 \n\
            var6[r10]:8 = INT_2COMP var5 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRode));
}
