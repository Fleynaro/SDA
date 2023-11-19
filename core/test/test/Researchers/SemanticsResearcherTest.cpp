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
            dataFlowRepo.get(),
            classRepo.get());
        semResearcher->addPropagator(
            std::make_unique<researcher::BaseSemanticsPropagator>(program, semRepo.get()));
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
            std::list<std::string> semList;
            for (auto semantics : obj.getSemantics()) {
                semList.push_back(semantics->getSemantics()->toString());
            }
            semList.sort();
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
        B0:var3, B0:var4 -> int32_t, int32_t, return \n\
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
        B0:var3, B0:var4, B3:var3, B3:var6 -> int32_t, int32_t, return, symbol_load(0x200:4) \n\
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
        B0:var1 -> empty \n\
        B0:var2 -> int32_t*, symbol_pointer \n\
        B0:var3, B0:var4, B7:var3 -> int32_t, param1, uint32_t \n\
        B0:var5 -> empty \n\
        B0:var6, B0:var8, Bb:var5 -> int32_t, return, uint32_t \n\
        B0:var7 -> symbol_pointer \n\
        B7:var1 -> empty \n\
        B7:var2 -> symbol_pointer \n\
        B7:var4, B7:var5, Bb:var3, Bb:var4 -> int32_t \n\
        Bb:var1 -> empty \n\
        Bb:var2 -> symbol_pointer \
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
