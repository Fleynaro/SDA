#include "Test/Core/IRcode/IRcodeFixture.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class IRcodeTest : public IRcodeFixture {};

TEST_F(IRcodeTest, Simplest) {
    // Graph: https://photos.app.goo.gl/qZr7FEV3H7bY9d9Q7
    auto sourcePCode = "\
        rax:8 = COPY 1:8 \n\
        rax:8 = INT_ADD rax:8, 1:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1[rax]:8 = COPY 0x1:8 \n\
            var2[rax]:8 = INT_ADD var1, 0x1:8 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
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
    auto expectedIRCode = "\
        Block B0(level: 1, far: B2): \n\
            var1[rax]:8 = COPY 0x2100:8 \n\
        Block B2(level: 2): \n\
            var2[rax]:1 = COPY 0x34:1 \n\
            var3:8 = REF var1 \n\
            var4:2 = EXTRACT var3, 0 \n\
            var5:2 = CONCAT var4, var2, 0 \n\
            var6[r10]:2 = INT_2COMP var5 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
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
    auto expectedIRCode = "\
        Block B0(level: 1, near: B2, far: B4, cond: 0x0:1): \n\
            var1[rax]:8 = COPY 0x0:8 \n\
        Block B2(level: 2, far: B5): \n\
            var2[rax]:8 = COPY 0x1:8 \n\
        Block B4(level: 2, near: B5): \n\
            empty \n\
        Block B5(level: 3): \n\
            var3:8 = REF var2 \n\
            var4:8 = REF var1 \n\
            var5:8 = PHI var3, var4 \n\
            var6[r10]:8 = INT_2COMP var5 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
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
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3, far: B5, cond: 0x0:1): \n\
            var1:8 = LOAD rsp \n\
            var2[rax]:8 = INT_ADD var1, 0x10:8 \n\
            var3[var2]:8 = COPY 0x0:8 \n\
        Block B3(level: 2, far: B6): \n\
            var4[rax]:8 = REF var2 \n\
            var5[var4]:8 = COPY 0x1:8 \n\
        Block B5(level: 2, near: B6): \n\
            empty \n\
        Block B6(level: 3): \n\
            var8[rax]:8 = REF var2 \n\
            var9[var8]:8 = REF var5 \n\
            var10[var8]:8 = REF var3 \n\
            var11[var8]:8 = PHI var9, var10 \n\
            var12[r10]:8 = COPY var11 \
    ";
    printVarAddressAlways = true;
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
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
    auto expectedIRCode = "\
        Block B0(level: 1, near: B2, far: B4, cond: 0x0:1): \n\
            empty \n\
        Block B2(level: 2, far: B5): \n\
            var1[rax]:1 = COPY 0x1:1 \n\
        Block B4(level: 2, near: B5): \n\
            var2[rax]:1 = COPY 0x2:1 \n\
        Block B5(level: 3): \n\
            var3:1 = REF var1 \n\
            var4:1 = REF var2 \n\
            var5:2 = LOAD rax \n\
            var6:2 = CONCAT var5, var3, 0 \n\
            var7:2 = CONCAT var5, var4, 1 \n\
            var8:2 = PHI var6, var7 \n\
            var9[r10]:2 = INT_2COMP var8 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
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
    auto expectedIRCode = "\
        Block B0(level: 1, near: B5, far: Ba, cond: 0x0:1): \n\
            var1[xmm0]:4 = COPY 0x1:4 \n\
            var2[xmm0]:4 = COPY 0x2:4 \n\
            var3[xmm0]:4 = COPY 0x3:4 \n\
            var4[xmm0]:4 = COPY 0x4:4 \n\
        Block B5(level: 2, far: Be): \n\
            var5:4 = REF var1 \n\
            var6[xmm0]:4 = INT_ADD var5, 0x1:4 \n\
            var7:4 = REF var2 \n\
            var8[xmm0]:4 = INT_ADD var7, 0x2:4 \n\
            var9:4 = REF var3 \n\
            var10[xmm0]:4 = INT_ADD var9, 0x3:4 \n\
            var11:4 = REF var4 \n\
            var12[xmm0]:4 = INT_ADD var11, 0x4:4 \n\
        Block Ba(level: 2, near: Be): \n\
            var13:4 = REF var1 \n\
            var14[xmm0]:4 = INT_SUB var13, 0x1:4 \n\
            var15:4 = REF var2 \n\
            var16[xmm0]:4 = INT_SUB var15, 0x2:4 \n\
            var17:4 = REF var3 \n\
            var18[xmm0]:4 = INT_SUB var17, 0x3:4 \n\
            var19:4 = REF var4 \n\
            var20[xmm0]:4 = INT_SUB var19, 0x4:4 \n\
        Block Be(level: 3): \n\
            var21:8 = LOAD rsp \n\
            var22[rax]:8 = INT_ADD var21, 0x0:8 \n\
            var23:4 = REF var6 \n\
            var24:4 = REF var14 \n\
            var25:4 = PHI var23, var24 \n\
            var26[var22]:4 = COPY var25 \n\
            var27[rax]:8 = INT_ADD var21, 0x4:8 \n\
            var28:4 = REF var8 \n\
            var29:4 = REF var16 \n\
            var30:4 = PHI var28, var29 \n\
            var31[var27]:4 = COPY var30 \n\
            var32[rax]:8 = INT_ADD var21, 0x8:8 \n\
            var33:4 = REF var10 \n\
            var34:4 = REF var18 \n\
            var35:4 = PHI var33, var34 \n\
            var36[var32]:4 = COPY var35 \n\
            var37[rax]:8 = INT_ADD var21, 0xc:8 \n\
            var38:4 = REF var12 \n\
            var39:4 = REF var20 \n\
            var40:4 = PHI var38, var39 \n\
            var41[var37]:4 = COPY var40 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
}

TEST_F(IRcodeTest, ReuseSameRef) {
    auto sourcePCode = "\
        rax:8 = COPY 0x1:8 \n\
        BRANCH <label> \n\
        <label>: \n\
        $0:8 = COPY rax:8 \n\
        $1:8 = COPY rax:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, far: B2): \n\
            var1[rax]:8 = COPY 0x1:8 \n\
        Block B2(level: 2): \n\
            var2:8 = REF var1 \n\
            var3[$U0]:8 = COPY var2 \n\
            var4[$U1]:8 = COPY var2 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
}

TEST_F(IRcodeTest, ReuseSamePhi) {
    auto sourcePCode = "\
        rax:8 = COPY 0:8 \n\
        CBRANCH <labelElse>, 0:1 // if-else condition \n\
        rax:8 = COPY 1:8 // then block \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        NOP // else block \n\
        <labelEnd>: \n\
        $1:8 = COPY rax:8 \n\
        $2:8 = COPY rax:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B2, far: B4, cond: 0x0:1): \n\
            var1[rax]:8 = COPY 0x0:8 \n\
        Block B2(level: 2, far: B5): \n\
            var2[rax]:8 = COPY 0x1:8 \n\
        Block B4(level: 2, near: B5): \n\
            empty \n\
        Block B5(level: 3): \n\
            var3:8 = REF var2 \n\
            var4:8 = REF var1 \n\
            var5:8 = PHI var3, var4 \n\
            var6[$U1]:8 = COPY var5 \n\
            var7[$U2]:8 = COPY var5 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
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
    auto expectedIRCode = "\
        Block B0(level: 1, near: B2, far: B8, cond: 0x0:1): \n\
            var1[rax]:8 = COPY 0x1:8 \n\
        Block B2(level: 2, near: B4, far: B6, cond: 0x0:1): \n\
            empty \n\
        Block B8(level: 2, near: B9): \n\
            empty \n\
        Block B4(level: 3, far: B9): \n\
            empty \n\
        Block B6(level: 3, far: B9): \n\
            empty \n\
        Block B9(level: 4): \n\
            var2:8 = REF var1 \n\
            var3[r10]:8 = INT_2COMP var2 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
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
    auto expectedIRCode = "\
        Block B0(level: 1, near: B2, far: B8, cond: 0x0:1): \n\
            empty \n\
        Block B2(level: 2, near: B4, far: B6, cond: 0x0:1): \n\
            var1[rax]:8 = COPY 0x0:8 \n\
        Block B8(level: 2, near: B9): \n\
            var2[rax]:8 = COPY 0x2:8 \n\
        Block B4(level: 3, far: B9): \n\
            var3[rax]:8 = COPY 0x1:8 \n\
        Block B6(level: 3, far: B9): \n\
            empty \n\
        Block B9(level: 4): \n\
            var4:8 = REF var3 \n\
            var5:8 = REF var2 \n\
            var6:8 = PHI var4, var5 \n\
            var7:8 = REF var1 \n\
            var8:8 = PHI var6, var7 \n\
            var9[r10]:8 = INT_2COMP var8 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
}

TEST_F(IRcodeTest, Loop) {
    // Graph: https://photos.app.goo.gl/ee1qx3J8rPKsBXa49
    auto sourcePCode = "\
        RAX:4 = COPY 0x1:4 \n\
        <labelLoop>: \n\
        RAX:4 = INT_ADD RAX:4, 0x1:4 \n\
        BRANCH <labelLoop> \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B1): \n\
            var1[rax]:4 = COPY 0x1:4 \n\
        Block B1(level: 2, far: B1): \n\
            var2:4 = REF var1 \n\
            var3:4 = REF var5 \n\
            var4:4 = PHI var2, var3 \n\
            var5[rax]:4 = INT_ADD var4, 0x1:4 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
}

TEST_F(IRcodeTest, LoopTwoVariable) {
    // Graph: https://photos.app.goo.gl/4zVVVLuVFSsUkSxK9
    auto sourcePCode = "\
        RAX:4 = COPY 0x1:4 \n\
        RCX:4 = COPY 0x1:4 \n\
        <labelLoop>: \n\
        RAX:4 = INT_ADD RAX:4, RCX:4 \n\
        BRANCH <labelJmp> \n\
        <labelJmp>: \n\
        RCX:4 = INT_ADD RCX:4, 0x1:4 \n\
        BRANCH <labelLoop> \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B2): \n\
            var1[rax]:4 = COPY 0x1:4 \n\
            var2[rcx]:4 = COPY 0x1:4 \n\
        Block B2(level: 2, far: B4): \n\
            var3:4 = REF var1 \n\
            var4:4 = REF var11 \n\
            var5:4 = PHI var3, var4 \n\
            var8:4 = REF var2 \n\
            var9:4 = REF var7 \n\
            var10:4 = PHI var8, var9 \n\
            var11[rax]:4 = INT_ADD var5, var10 \n\
        Block B4(level: 3, far: B2): \n\
            var6:4 = REF var10 \n\
            var7[rcx]:4 = INT_ADD var6, 0x1:4 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
}

TEST_F(IRcodeTest, NestedLoop) {
    // Graph: https://photos.app.goo.gl/jF3aeZGeaW5LhcGh9
    auto sourcePCode = "\
        RAX:4 = COPY 0x1:4 \n\
        <labelLoop>: \n\
        RAX:4 = INT_ADD RAX:4, 0x1:4 \n\
        CBRANCH <labelLoop>, 0:1 \n\
        RAX:4 = INT_MULT RAX:4, 0x2:4 \n\
        BRANCH <labelLoop> \n\
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B1): \n\
            var1[rax]:4 = COPY 0x1:4 \n\
        Block B1(level: 2, near: B3, far: B1, cond: 0x0:1): \n\
            var2:4 = REF var1 \n\
            var3:4 = REF var7 \n\
            var4:4 = PHI var2, var3 \n\
            var5:4 = REF var9 \n\
            var6:4 = PHI var4, var5 \n\
            var7[rax]:4 = INT_ADD var6, 0x1:4 \n\
        Block B3(level: 3, far: B1): \n\
            var8:4 = REF var7 \n\
            var9[rax]:4 = INT_MULT var8, 0x2:4 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
}

TEST_F(IRcodeTest, TwoEntryPointsUnion) {
    // Graph: https://photos.app.goo.gl/vVGiX5e3cDuH8q84A
    auto sourcePCode = "\
        RAX:4 = COPY 0x1:4 \n\
        BRANCH <label> \n\
        RAX:4 = COPY 0x2:4 \n\
        BRANCH <label> \n\
        <label>: \n\
        RAX:4 = INT_ADD RAX:4, 0x1:4 \
    ";
    auto expectedIRCodeOfFunc1 = "\
        Block B0(level: 1): \n\
            var1[rax]:4 = COPY 0x1:4 \
    ";
    auto expectedIRCodeOfFunc2 = "\
        Block B2(level: 1): \n\
            var1[rax]:4 = COPY 0x2:4 \
    ";
    auto expectedIRCodeOfFunc3 = "\
        Block B4(level: 1): \n\
            var1:4 = LOAD rax \n\
            var2[rax]:4 = INT_ADD var1, 0x1:4 \
    ";
    auto instructions = PcodeFixture::parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph->explore(pcode::InstructionOffset(0, 0), &provider);
    graph->explore(pcode::InstructionOffset(2, 0), &provider);
    auto func1 = program->toFunction(graph->getFunctionGraphAt(pcode::InstructionOffset(0, 0)));
    auto func2 = program->toFunction(graph->getFunctionGraphAt(pcode::InstructionOffset(2, 0)));
    auto func3 = program->toFunction(graph->getFunctionGraphAt(pcode::InstructionOffset(4, 0)));
    ASSERT_TRUE(cmp(func1, expectedIRCodeOfFunc1));
    ASSERT_TRUE(cmp(func2, expectedIRCodeOfFunc2));
    ASSERT_TRUE(cmp(func3, expectedIRCodeOfFunc3));
}
