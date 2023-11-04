#include "Test/Core/Researchers/DataFlowResearcherFixture.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class DataFlowResearcherTest : public DataFlowResearcherFixture {};

TEST_F(DataFlowResearcherTest, GlobalVarAssignment) {
    /*
        float func(float param1) {
            globalVar_0x10 = param1;
            return globalVar_0x10;
        }
    */
    auto sourcePCode = "\
        r10:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE r10:8, xmm0:Da \n\
        rax:4 = LOAD r10:8, 4:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm0 \n\
            var4[var2]:4 = COPY var3 \n\
            var5[rax]:4 = COPY var4 \
    ";
    auto expectedDataFlow = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Unknown \n\
        var4 <- Write var2 \n\
        var4 <- Write var3 \n\
        var5 <- Copy var4 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowResearcherTest, GlobalVarAssignmentDouble) {
    /*
        float func(float param1, float param2) {
            globalVar_0x10 = param1;
            globalVar_0x18 = param2;
            return globalVar_0x18;
        }
    */
    auto sourcePCode = "\
        r10:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE r10:8, xmm0:Da \n\
        r10:8 = INT_ADD rip:8, 0x18:8 \n\
        STORE r10:8, xmm1:Da \n\
        rax:4 = LOAD r10:8, 4:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm0 \n\
            var4[var2]:4 = COPY var3 \n\
            var5[r10]:8 = INT_ADD var1, 0x18:8 \n\
            var6:4 = LOAD xmm1 \n\
            var7[var5]:4 = COPY var6 \n\
            var8[rax]:4 = COPY var7 \
    ";
    auto expectedDataFlow = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Unknown \n\
        var4 <- Write var2 \n\
        var4 <- Write var3 \n\
        var5 <- Copy var1 + 0x18 \n\
        var6 <- Unknown \n\
        var7 <- Write var5 \n\
        var7 <- Write var6 \n\
        var8 <- Copy var7 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowResearcherTest, GlobalVarAssignmentObject) {
    /*
        void func(Object* param1, float param2) {
            param1->field_0x10 = param2;
            globalVar_0x200 = param1;
        }
    */
    auto sourcePCode = "\
        r10:8 = INT_ADD rcx:8, 0x10:8 \n\
        STORE r10:8, xmm1:Da \n\
        r10:8 = INT_ADD rip:8, 0x200:8 \n\
        STORE r10:8, rcx:8 \n\
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rcx \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm1 \n\
            var4[var2]:4 = COPY var3 \n\
            var5:8 = LOAD rip \n\
            var6[r10]:8 = INT_ADD var5, 0x200:8 \n\
            var7[var6]:8 = COPY var1 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Unknown \n\
        var4 <- Write var2 \n\
        var4 <- Write var3 \n\
        var5 <- Copy Start \n\
        var6 <- Copy var5 + 0x200 \n\
        var7 <- Write var6 \n\
        var7 <- Write var1 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowResearcherTest, GlobalVarAssignmentObjectDouble) {
    /*
        void func(Object* param1, float param2) {
            param1->field_0x10 = param2;
            globalVar_0x200 = param1;
            globalVar_0x208 = param1;
        }
    */
    auto sourcePCode = "\
        r10:8 = INT_ADD rcx:8, 0x10:8 \n\
        STORE r10:8, xmm1:Da \n\
        r10:8 = INT_ADD rip:8, 0x200:8 \n\
        STORE r10:8, rcx:8 \n\
        r10:8 = INT_ADD rip:8, 0x208:8 \n\
        STORE r10:8, rcx:8 \n\
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rcx \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm1 \n\
            var4[var2]:4 = COPY var3 \n\
            var5:8 = LOAD rip \n\
            var6[r10]:8 = INT_ADD var5, 0x200:8 \n\
            var7[var6]:8 = COPY var1 \n\
            var8[r10]:8 = INT_ADD var5, 0x208:8 \n\
            var9[var8]:8 = COPY var1 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Unknown \n\
        var4 <- Write var2 \n\
        var4 <- Write var3 \n\
        var5 <- Copy Start \n\
        var6 <- Copy var5 + 0x200 \n\
        var7 <- Write var6 \n\
        var7 <- Write var1 \n\
        var8 <- Copy var5 + 0x208 \n\
        var9 <- Write var8 \n\
        var9 <- Write var1 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowResearcherTest, If) {
    /*
        void func() {
            ...
            result = var10;
            if (param1 == 5) {
                result = var11;
            }
        }
    */
    auto sourcePCode = "\
        rax:8 = COPY $10:8 \n\
        $1:1 = INT_NOTEQUAL rcx:4, 5:4 \n\
        CBRANCH <label>, $1:1 \n\
        rax:8 = COPY $11:8 \n\
        <label>: \n\
        r10:8 = INT_2COMP rax:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3, far: B4, cond: var4): \n\
            var1:8 = LOAD $U10 \n\
            var2[rax]:8 = COPY var1 \n\
            var3:4 = LOAD rcx \n\
            var4[$U1]:1 = INT_NOTEQUAL var3, 0x5:4 \n\
        Block B3(level: 2, near: B4): \n\
            var5:8 = LOAD $U11 \n\
            var6[rax]:8 = COPY var5 \n\
        Block B4(level: 3): \n\
            var7:8 = REF var2 \n\
            var8:8 = REF var6 \n\
            var9:8 = PHI var7, var8 \n\
            var10[r10]:8 = INT_2COMP var9 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Copy var1 \n\
        var5 <- Unknown \n\
        var6 <- Copy var5 \n\
        var7 <- Copy var2 \n\
        var8 <- Copy var6 \n\
        var9 <- Copy var7 \n\
        var9 <- Copy var8 \
    ";
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowResearcherTest, IfConditionMem) {
    /*
        float func() {
            localVar_0x10 = 0;
            if (!0) {
                localVar_0x10 = 1;
            }
            return localVar_0x10;
        }
    */
    auto sourcePCode = "\
        $0:8 = INT_ADD rsp:8, 0x10:8 \n\
        STORE $0:8, 0x0:8 \n\
        CBRANCH <labelEnd>, 0:1 // if-else condition \n\
        STORE $0:8, 1:8 // then block \n\
        <labelEnd>: \n\
        rax:8 = LOAD $0:8, 8:8 \
    ";
    // [var8] is virtual address memory here and ignored by data flow research
    auto expectedIRCode = "\
        Block B0(level: 1, near: B3, far: B4, cond: 0x0:1): \n\
            var1:8 = LOAD rsp \n\
            var2[$U0]:8 = INT_ADD var1, 0x10:8 \n\
            var3[var2]:8 = COPY 0x0:8 \n\
        Block B3(level: 2, near: B4): \n\
            var4[$U0]:8 = REF var2 \n\
            var5[var4]:8 = COPY 0x1:8 \n\
        Block B4(level: 3): \n\
            var8[$U0]:8 = REF var2 \n\
            var9[var8]:8 = REF var3 \n\
            var10[var8]:8 = REF var5 \n\
            var11[var8]:8 = PHI var9, var10 \n\
            var12[rax]:8 = COPY var11 \
    ";
    auto expectedDataFlow = "\
        var1 <- Unknown \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Write var2 \n\
        var3 <- Write 0x0 \n\
        var4 <- Copy var2 \n\
        var5 <- Write var4 \n\
        var5 <- Write 0x1 \n\
        var8 <- Copy var2 \n\
        var9 <- Copy var3 \n\
        var10 <- Copy var5 \n\
        var11 <- Copy var9 \n\
        var11 <- Copy var10 \n\
        var12 <- Copy var11 \
    ";
    printVarAddressAlways = true;
    auto function = parsePcode(sourcePCode, program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    ASSERT_TRUE(cmpDataFlow(function, expectedDataFlow));
}

TEST_F(DataFlowResearcherTest, Functions) {
    /*
        void main() {
            setGlobalFloatValue(0.5);
        }

        bool setGlobalFloatValue(float value) {
            globalVar_0x10 = value;
            return true;
        }
    */
    auto sourcePCode = "\
        // main() \n\
        xmm0:Da = COPY 0.5:4 \n\
        CALL <setGlobalFloatValue> \n\
        RETURN \n\
        \n\
        \n\
        // bool setGlobalFloatValue(float value) \n\
        <setGlobalFloatValue>: \n\
        $0:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE $0:8, xmm0:Da \n\
        rax:1 = COPY 0x1:1 \n\
        RETURN \n\
    ";
    auto setGlobalFloatValueSig = "\
        setGlobalFloatValueSig = signature fastcall bool( \
            float param1 \
        ) \
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1): \n\
            var1[xmm0]:4 = COPY 0x0:4 \n\
            var2[rax]:1 = CALL 0x300:8, var1 \
    ";
    auto expectedDataFlowOfMainFunc = "\
        var1 <- Copy 0x0 \n\
        var2 <- Copy B3:var5 \n\
        B3:var3 <- Copy var1 \
    ";
    auto expectedIRCodeOfFunc2 = "\
        Block B3(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U0]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm0 // param1 \n\
            var4[var2]:4 = COPY var3 \n\
            var5[rax]:1 = COPY 0x1:1 // return \
    ";
    auto expectedDataFlowOfFunc2 = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x10 \n\
        var3 <- Copy B0:var1 \n\
        var4 <- Write var2 \n\
        var4 <- Write var3 \n\
        var5 <- Copy 0x1 \n\
        B0:var2 <- Copy var5 \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto setGlobalFloatValueSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(setGlobalFloatValueSig));
    auto func2 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(3, 0)));
    func2->getFunctionSymbol()->getSignature()->copyFrom(setGlobalFloatValueSigDt);
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmp(func2, expectedIRCodeOfFunc2));
    ASSERT_TRUE(cmpDataFlow(mainFunction, expectedDataFlowOfMainFunc));
    ASSERT_TRUE(cmpDataFlow(func2, expectedDataFlowOfFunc2));
}
