#include "Test/Core/Researchers/ClassResearcherFixture.h"
#include "SDA/Core/Researchers/SemanticsResearcher.h"
#include <boost/algorithm/string/join.hpp>

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class SemanticsResearcherTest : public ClassResearcherFixture
{
protected:
    std::unique_ptr<researcher::SemanticsRepository> semRepo;
    std::unique_ptr<researcher::SemanticsResearcher> semResearcher;

    void SetUp() override {
        ClassResearcherFixture::SetUp();
        semRepo = std::make_unique<researcher::SemanticsRepository>(eventPipe);
        semResearcher = std::make_unique<researcher::SemanticsResearcher>(
            program,
            semRepo.get(),
            classRepo.get(),
            dataFlowRepo.get());
        semResearcher->addPropagator(
            std::make_unique<researcher::BaseSemanticsPropagator>(
                program,
                semRepo.get(),
                dataFlowRepo.get()));
        eventPipe->connect(semResearcher->getEventPipe());
    }

    ::testing::AssertionResult cmpSemantics(const std::string& expectedCode) const {
        std::list<std::string> list;
        for (auto& obj : semRepo->getObjects()) {
            std::list<std::string> varList;
            for (auto& var : obj.getVariables()) {
                varList.push_back(var->getName(true));
            }
            varList.sort();
            auto varListStr = boost::algorithm::join(varList, ", ");
            std::map<std::string, size_t> semMap;
            for (auto semantics : obj.getSemantics()) {
                auto semStr = semantics->getSemantics()->toString();
                if (semMap.find(semStr) == semMap.end()) {
                    semMap[semStr] = 1;
                } else {
                    semMap[semStr]++;
                }
            }
            std::list<std::string> semList;
            for (auto& [semStr, count] : semMap) {
                if (count == 1) {
                    semList.push_back(semStr);
                } else {
                    semList.push_back(semStr + " x " + std::to_string(count));
                }
            }
            auto semListStr = boost::algorithm::join(semList, ", ");
            if (semListStr.empty()) {
                list.push_back(varListStr + " -> empty");
            } else {
                list.push_back(varListStr + " -> " + semListStr);
            }
        }
        list.sort();
        return Compare(boost::algorithm::join(list, "\n"), expectedCode);
    }
};

TEST_F(SemanticsResearcherTest, Simple1) {
    /*
        int func(int param1) {
            return param1 + 1;
        }
    */
   auto sourcePCode = "\
        // int func(int param1) \n\
        $0:4 = COPY rcx:4 \n\
        $1:4 = INT_ADD $0:4, 0x1:4 \n\
        rax:4 = COPY $1:4 \n\
        RETURN \n\
    ";
    auto funcSig = "\
        funcSig = signature fastcall int32_t(int32_t param1) \
    ";
    auto expectedIRCodeOfFunc = "\
        Block B0(level: 1): \n\
            var1:4 = LOAD rcx // param1 \n\
            var2[$U0]:4 = COPY var1 \n\
            var3[$U1]:4 = INT_ADD var2, 0x1:4 \n\
            var4[rax]:4 = COPY var3 // return \
    ";
    auto expectedSemantics = "\
        B0:var1, B0:var2 -> int32_t, param1 \n\
        B0:var3, B0:var4 -> int32_t x 2, return \n\
    ";
    auto func = parsePcode(sourcePCode, program);
    auto funcSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(funcSig));
    func->getFunctionSymbol()->getSignature()->copyFrom(funcSigDt);
    ASSERT_TRUE(cmp(func, expectedIRCodeOfFunc));
    ASSERT_TRUE(cmpSemantics(expectedSemantics));
}

TEST_F(SemanticsResearcherTest, Simple2) {
    /*
        int func1() {
            return globalVar_0x200;
        }

        void func2(int param1) {
            globalVar_0x200 = param1 + 1;
        }
    */
   auto sourcePCode = "\
        // int func1() \n\
        $0:8 = INT_ADD rip:8, 0x200:8 \n\
        rax:4 = LOAD $0:8, 4:8 \n\
        RETURN \n\
        \n\
        \n\
        // void func2(int param1) \n\
        $0:4 = COPY rcx:4 \n\
        $1:4 = INT_ADD $0:4, 0x1:4 \n\
        $2:8 = INT_ADD rip:8, 0x200:8 \n\
        STORE $2:8, $1:4 \n\
        RETURN \n\
    ";
    auto func1Sig = "\
        func1Sig = signature fastcall int32_t() \
    ";
    auto func2Sig = "\
        func2Sig = signature fastcall void(int32_t param1) \
    ";
    auto expectedIRCodeOfFunc1 = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U0]:8 = INT_ADD var1, 0x200:8 \n\
            var3:4 = LOAD var2 \n\
            var4[rax]:4 = COPY var3 // return \
    ";
    auto expectedIRCodeOfFunc2 = "\
        Block B3(level: 1): \n\
            var1:4 = LOAD rcx // param1 \n\
            var2[$U0]:4 = COPY var1 \n\
            var3[$U1]:4 = INT_ADD var2, 0x1:4 \n\
            var4:8 = LOAD rip \n\
            var5[$U2]:8 = INT_ADD var4, 0x200:8 \n\
            var6[var5]:4 = COPY var3 \
    ";
    auto expectedSemantics = "\
        B0:var1, B3:var4 -> symbol_pointer(0x0) \n\
        B0:var2, B3:var5 -> symbol_pointer(0x200) \n\
        B0:var3, B0:var4, B3:var3, B3:var6 -> int32_t x 2, return, symbol_load(0x200:4) \n\
        B3:var1, B3:var2 -> int32_t, param1 \
    ";
    auto instructions = PcodeFixture::parsePcode(sourcePCode);
    pcode::ListInstructionProvider provider(instructions);
    graph->explore(pcode::InstructionOffset(0, 0), &provider);
    graph->explore(pcode::InstructionOffset(3, 0), &provider);
    auto func1SigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(func1Sig));
    auto func2SigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(func2Sig));
    auto func1 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(0, 0)));
    auto func2 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(3, 0)));
    func1->getFunctionSymbol()->getSignature()->copyFrom(func1SigDt);
    func2->getFunctionSymbol()->getSignature()->copyFrom(func2SigDt);
    ASSERT_TRUE(cmp(func1, expectedIRCodeOfFunc1));
    ASSERT_TRUE(cmp(func2, expectedIRCodeOfFunc2));
    ASSERT_TRUE(cmpSemantics(expectedSemantics));
}

TEST_F(SemanticsResearcherTest, Simple3) {
    /*
        // Description: we set int32_t for globalVar_0x100 and should get int32_t for globalVar_0x300

        void main() {
            func1(globalVar_0x100);
            globalVar_0x300 = func2();
        }

        void func1(uint32_t param1) {
            globalVar_0x200 = param1 + 1;
        }

        uint32_t func2() {
            return globalVar_0x200 + 10;
        }
    */
   auto sourcePCode = "\
        // main() \n\
        $1:8 = INT_ADD rip:8, 0x100:8 \n\
        rcx:4 = LOAD $1:8, 4:8 \n\
        CALL <func1> \n\
        CALL <func2> \n\
        $2:8 = INT_ADD rip:8, 0x300:8 \n\
        STORE $2:8, rax:4 \n\
        RETURN \n\
        \n\
        \n\
        // func1(int param1) \n\
        <func1>: \n\
        $1:8 = INT_ADD rip:8, 0x200:8 \n\
        $2:4 = INT_ADD rcx:4, 0x1:4 \n\
        STORE $1:8, $2:4 \n\
        RETURN \n\
        \n\
        \n\
        // int func2() \n\
        <func2>: \n\
        $1:8 = INT_ADD rip:8, 0x200:8 \n\
        $2:4 = LOAD $1:8, 4:8 \n\
        rax:4 = INT_ADD $2:4, 10:4 \n\
        RETURN \
    ";
    auto func1Sig = "\
        func1Sig = signature fastcall void(uint32_t param1) \
    ";
    auto func2Sig = "\
        func2Sig = signature fastcall uint32_t() \
    ";
    auto globalSymbolTableCode = "\
        { \
            int32_t globalVar_0x100 = 0x100 \
        } \
    ";
    auto expectedIRCodeOfMain = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x100:8 \n\
            var3:4 = LOAD var2 \n\
            var4[rcx]:4 = COPY var3 \n\
            var5:1 = CALL 0x700:8, var4 \n\
            var6[rax]:4 = CALL 0xb00:8 \n\
            var7[$U2]:8 = INT_ADD var1, 0x300:8 \n\
            var8[var7]:4 = COPY var6 \
    ";
    auto expectedIRCodeOfFunc1 = "\
        Block B7(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x200:8 \n\
            var3:4 = LOAD rcx // param1 \n\
            var4[$U2]:4 = INT_ADD var3, 0x1:4 \n\
            var5[var2]:4 = COPY var4 \
    ";
    auto expectedIRCodeOfFunc2 = "\
        Block Bb(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x200:8 \n\
            var3:4 = LOAD var2 \n\
            var4[$U2]:4 = COPY var3 \n\
            var5[rax]:4 = INT_ADD var4, 0xa:4 // return \
    ";
    auto expectedSemantics = "\
        B0:var1, B7:var1, Bb:var1 -> symbol_pointer(0x0) \n\
        B0:var2 -> symbol_pointer(0x100) \n\
        B0:var3, B0:var4, B7:var3 -> int32_t, param1, symbol_load(0x100:4), uint32_t \n\
        B0:var5 -> empty \n\
        B0:var6, B0:var8, Bb:var5 -> int32_t, return, symbol_load(0x300:4), uint32_t \n\
        B0:var7 -> symbol_pointer(0x300) \n\
        B7:var2, Bb:var2 -> symbol_pointer(0x200) \n\
        B7:var4, B7:var5, Bb:var3, Bb:var4 -> int32_t, symbol_load(0x200:4) \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto func1SigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(func1Sig));
    auto func2SigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(func2Sig));
    auto func1 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(7, 0)));
    auto func2 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(11, 0)));
    func1->getFunctionSymbol()->getSignature()->copyFrom(func1SigDt);
    func2->getFunctionSymbol()->getSignature()->copyFrom(func2SigDt);
    {
        // add symbol at 0x100 offset (see globalSymbolTableCode)
        parseSymbolTable(globalSymbolTableCode, false, globalSymbolTable);
        // remove symbol
        globalSymbolTable->removeSymbol(0x100);
        // add the same symbol again
        parseSymbolTable(globalSymbolTableCode, false, globalSymbolTable);
    }
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMain));
    ASSERT_TRUE(cmp(func1, expectedIRCodeOfFunc1));
    ASSERT_TRUE(cmp(func2, expectedIRCodeOfFunc2));
    ASSERT_TRUE(cmpSemantics(expectedSemantics));
}

TEST_F(SemanticsResearcherTest, GlobalVarAssignmentObject) {
    /*
        // Description: we set TestStruct* for globalVar_0x200 and should get float for param2

        void func(TestStruct* param1, float param2) {
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
    auto testStructureCode = "\
        TestStruct = struct { \
            float b = 0x10 \
        } \
    ";
    auto globalSymbolTableCode = "\
        { \
            TestStruct* globalVar_0x200 = 0x200 \
        } \
    ";
    auto expectedIRCodeOfFunc = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rcx \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm1 \n\
            var4[var2]:4 = COPY var3 \n\
            var5:8 = LOAD rip \n\
            var6[r10]:8 = INT_ADD var5, 0x200:8 \n\
            var7[var6]:8 = COPY var1 \
    ";
    auto expectedSemantics = "\
        B0:var1, B0:var7 -> TestStruct*, symbol_load(0x200:8), symbol_pointer(0x0) \n\
        B0:var2 -> symbol_pointer(0x10) \n\
        B0:var3, B0:var4 -> float, symbol_load(0x10:4) \n\
        B0:var5 -> symbol_pointer(0x0) \n\
        B0:var6 -> symbol_pointer(0x200) \
    ";
    auto func = parsePcode(sourcePCode, program);
    parseDataType(testStructureCode);
    parseSymbolTable(globalSymbolTableCode, false, globalSymbolTable);
    ASSERT_TRUE(cmp(func, expectedIRCodeOfFunc));
    ASSERT_TRUE(cmpSemantics(expectedSemantics));
}

TEST_F(SemanticsResearcherTest, IfElseCondition) {
    /*
        void func(int32_t param1, int32_t param2) {
            if (0) {
                return param2;
            }
            else {
                return param1;
            }
        }
    */
    auto sourcePCode = "\
        CBRANCH <labelElse>, 0:1 // if-else condition \n\
        $0:4  = COPY rdx:4 // then block \n\
        BRANCH <labelEnd> \n\
        <labelElse>: \n\
        $0:4  = COPY rcx:4 // else block \n\
        <labelEnd>: \n\
        rax:4 = COPY $0:4 // return \
    ";
    auto funcSig = "\
        funcSig = signature fastcall void(int32_t param1, int32_t param2) \
    ";
    auto expectedIRCodeOfFunc = "\
        Block B0(level: 1, near: B1, far: B3, cond: 0x0:1): \n\
            empty \n\
        Block B1(level: 2, far: B4): \n\
            var1:4 = LOAD rdx // param2 \n\
            var2[$U0]:4 = COPY var1 \n\
        Block B3(level: 2, near: B4): \n\
            var3:4 = LOAD rcx // param1 \n\
            var4[$U0]:4 = COPY var3 \n\
        Block B4(level: 3): \n\
            var5:4 = REF var2 \n\
            var6:4 = REF var4 \n\
            var7:4 = PHI var5, var6 \n\
            var8[rax]:4 = COPY var7 \
    ";
    auto expectedSemantics = "\
        B0:var1, B0:var2, B0:var3, B0:var4, B0:var5, B0:var6, B0:var7, B0:var8 -> int32_t x 2, param1, param2 \
    ";
    auto func = parsePcode(sourcePCode, program);
    auto funcSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(funcSig));
    func->getFunctionSymbol()->getSignature()->copyFrom(funcSigDt);
    ASSERT_TRUE(cmp(func, expectedIRCodeOfFunc));
    ASSERT_TRUE(cmpSemantics(expectedSemantics));
}

TEST_F(SemanticsResearcherTest, CopyObjectInParts) {
    /*
        // Description: we set TestStruct for globalVar_0x100 and should get TestStruct* for param1

        void func(TestStruct* param1) {
            *param1 = globalVar_0x100;
        }
    */
   auto sourcePCode = "\
        $0:8 = INT_ADD rip:8, 0x100:8 \n\
        $1:8 = INT_ADD $0:8, 0x8:8 \n\
        $2:8 = LOAD $0:8, 8:8 \n\
        $3:8 = LOAD $1:8, 8:8 \n\
        $4:8 = INT_ADD rcx:8, 0x10:8 \n\
        $5:8 = INT_ADD $4:8, 0x8:8 \n\
        STORE $4:8, $2:8 \n\
        STORE $5:8, $3:8 \
    ";
    auto testStructureCode = "\
        TestStruct = struct { \
            float x, \
            float y, \
            float z, \
            float w, \
        } \
    ";
    auto globalSymbolTableCode = "\
        { \
            TestStruct globalVar_0x100 = 0x100 \
        } \
    ";
    auto expectedIRCodeOfFunc = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U0]:8 = INT_ADD var1, 0x100:8 \n\
            var3[$U1]:8 = INT_ADD var2, 0x8:8 \n\
            var4:8 = LOAD var2 \n\
            var5[$U2]:8 = COPY var4 \n\
            var6:8 = LOAD var3 \n\
            var7[$U3]:8 = COPY var6 \n\
            var8:8 = LOAD rcx \n\
            var9[$U4]:8 = INT_ADD var8, 0x10:8 \n\
            var10[$U5]:8 = INT_ADD var9, 0x8:8 \n\
            var11[var9]:8 = COPY var5 \n\
            var12[var10]:8 = COPY var7 \
    ";
    auto expectedSemantics = "\
        B0:var1 -> symbol_pointer(0x0) \n\
        B0:var10 -> empty \n\
        B0:var11, B0:var4, B0:var5 -> symbol_load(0x0:8), symbol_load(0x100:8) \n\
        B0:var12, B0:var6, B0:var7 -> symbol_load(0x108:8), symbol_load(0x8:8) \n\
        B0:var2 -> symbol_pointer(0x0), symbol_pointer(0x100) \n\
        B0:var3 -> symbol_pointer(0x108), symbol_pointer(0x8) \n\
        B0:var8 -> empty \n\
        B0:var9 -> empty \
    ";
    auto func = parsePcode(sourcePCode, program);
    parseDataType(testStructureCode);
    parseSymbolTable(globalSymbolTableCode, false, globalSymbolTable);
    ASSERT_TRUE(cmp(func, expectedIRCodeOfFunc));
    ASSERT_TRUE(cmpSemantics(expectedSemantics));
}

TEST_F(SemanticsResearcherTest, MutualFunction) {
    /*
        void main() {
            if (globalVar_0x100->field_0x0 == 1) {
                player = static_cast<Player*>(globalVar_0x100);
                mutual(player, 1);
            }
            else if (globalVar_0x100->field_0x0 == 2) {
                vehicle = static_cast<Vehicle*>(globalVar_0x100);
                mutual(vehicle, 2);
            }
        }

        void mutual(Entity* entity, int value) {
            entity->field_0x8 = value;
        }
    */
   auto sourcePCode = "\
        // main() \n\
        $1:8 = INT_ADD rip:8, 0x100:8 \n\
        $2:8 = LOAD $1:8, 8:8 \n\
        $3:4 = LOAD $2:8, 4:8 \n\
        $4:1 = INT_NOTEQUAL $3:4, 1:4 \n\
        CBRANCH <vehicle_check>, $4:1 \n\
        rcx:8 = COPY $2:8 \n\
        rdx:4 = COPY 0x1:4 \n\
        CALL <mutual> \n\
        BRANCH <end> \n\
        <vehicle_check>: \n\
        $5:4 = LOAD $2:8, 4:8 \n\
        $6:1 = INT_NOTEQUAL $5:4, 2:4 \n\
        CBRANCH <end>, $6:1 \n\
        rcx:8 = COPY $2:8 \n\
        rdx:4 = COPY 0x2:4 \n\
        CALL <mutual> \n\
        <end>: \n\
        RETURN \n\
        \n\
        \n\
        // mutual(Entity* entity, int value) \n\
        <mutual>: \n\
        $1:8 = INT_ADD rcx:8, 0x8:8 \n\
        STORE $1:8, rdx:4 \n\
        RETURN \
    ";
    auto mutualSig = "\
        mutualSig = signature fastcall void(uint64_t param1, uint32_t param2) \
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1, near: B5, far: B9, cond: var7): \n\
            var1:8 = LOAD rip \n\
            var2[$U1]:8 = INT_ADD var1, 0x100:8 \n\
            var3:8 = LOAD var2 \n\
            var4[$U2]:8 = COPY var3 \n\
            var5:4 = LOAD var4 \n\
            var6[$U3]:4 = COPY var5 \n\
            var7[$U4]:1 = INT_NOTEQUAL var6, 0x1:4 \n\
        Block B5(level: 2, far: Bf): \n\
            var8:8 = REF var4 \n\
            var9[rcx]:8 = COPY var8 \n\
            var10[rdx]:4 = COPY 0x1:4 \n\
            var11:1 = CALL 0x1000:8, var9, var10 \n\
        Block B9(level: 2, near: Bc, far: Bf, cond: var15): \n\
            var12:8 = REF var4 \n\
            var13:4 = REF var5 \n\
            var14[$U5]:4 = COPY var13 \n\
            var15[$U6]:1 = INT_NOTEQUAL var14, 0x2:4 \n\
        Block Bc(level: 3, near: Bf): \n\
            var16:8 = REF var12 \n\
            var17[rcx]:8 = COPY var16 \n\
            var18[rdx]:4 = COPY 0x2:4 \n\
            var19:1 = CALL 0x1000:8, var17, var18 \n\
        Block Bf(level: 4): \n\
            empty \
    ";
    auto expectedIRCodeOfMutualFunc = "\
        Block B10(level: 1): \n\
            var1:8 = LOAD rcx // param1 \n\
            var2[$U1]:8 = INT_ADD var1, 0x8:8 \n\
            var3:4 = LOAD rdx // param2 \n\
            var4[var2]:4 = COPY var3 \
    ";
    auto expectedSemantics = "\
        B0:var1 -> symbol_pointer(0x0) \n\
        B0:var10 -> param2, uint32_t \n\
        B0:var11 -> empty \n\
        B0:var12, B0:var3, B0:var4, B10:var1 -> class_label_field(0x0) x 2, param1 x 3, symbol_load(0x100:8), uint64_t x 3 \n\
        B0:var13, B0:var14, B0:var5, B0:var6 -> empty \n\
        B0:var15 -> bool, operation \n\
        B0:var16, B0:var17 -> param1, uint64_t \n\
        B0:var18 -> param2, uint32_t \n\
        B0:var19 -> empty \n\
        B0:var2 -> symbol_pointer(0x100) \n\
        B0:var7 -> bool, operation \n\
        B0:var8, B0:var9 -> param1, uint64_t \n\
        B10:var2 -> empty \n\
        B10:var3, B10:var4 -> param2 x 3, uint32_t x 3 \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto mutualSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(mutualSig));
    auto mutualFunc = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(0x10, 0)));
    mutualFunc->getFunctionSymbol()->getSignature()->copyFrom(mutualSigDt);
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmp(mutualFunc, expectedIRCodeOfMutualFunc));
    ASSERT_TRUE(cmpSemantics(expectedSemantics));
}

TEST_F(SemanticsResearcherTest, CyclicPropagation) {
    /*
        // Description: int32_t data type should not be propagated cyclically (see INT_MULT)

        void func(int param1) {
            stack_0x0 = param1;
            stack_0x0 = param1 * 50;
        }
    */
   auto sourcePCode = "\
        // int func(int param1) \n\
        $0:4 = COPY rcx:4 \n\
        STORE rsp:8, $0:4 \n\
        $1:4 = INT_MULT $0:4, 50:4 \n\
        STORE rsp:8, $1:4 \n\
        RETURN \n\
    ";
    auto funcSig = "\
        funcSig = signature fastcall void(int32_t param1) \
    ";
    auto expectedIRCodeOfFunc = "\
        Block B0(level: 1): \n\
            var1:4 = LOAD rcx // param1 \n\
            var2[$U0]:4 = COPY var1 \n\
            var3:8 = LOAD rsp \n\
            var4[var3]:4 = COPY var2 \n\
            var5[$U1]:4 = INT_MULT var2, 0x32:4 \n\
            var6[var3]:4 = COPY var5 \
    ";
    auto expectedSemantics = "\
        B0:var1, B0:var2, B0:var4, B0:var5, B0:var6 -> int32_t, param1, symbol_load(0x0:4) \n\
        B0:var3 -> symbol_pointer(0x0) \
    ";
    auto func = parsePcode(sourcePCode, program);
    auto funcSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(funcSig));
    func->getFunctionSymbol()->getSignature()->copyFrom(funcSigDt);
    ASSERT_TRUE(cmp(func, expectedIRCodeOfFunc));
    ASSERT_TRUE(cmpSemantics(expectedSemantics));
}
