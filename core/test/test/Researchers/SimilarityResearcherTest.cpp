#include "Test/Core/Researchers/ClassResearcherFixture.h"
#include "SDA/Core/Researchers/SimilarityResearcher.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class SimilarityResearcherTest : public ClassResearcherFixture
{
protected:
    std::unique_ptr<researcher::SimilarityResearcher> simResearcher;

    void SetUp() override {
        ClassResearcherFixture::SetUp();
        simResearcher = std::make_unique<researcher::SimilarityResearcher>(
            classRepo.get(),
            structureRepo.get(),
            dataFlowRepo.get());
    }

    ::testing::AssertionResult cmpVariables(const std::set<std::shared_ptr<ircode::Variable>>& vars, const std::string& expectedCode) const {
        std::stringstream ss;
        std::list<std::shared_ptr<ircode::Variable>> varList;
        for (auto var : vars) {
            varList.push_back(var);
        }
        varList.sort([](const std::shared_ptr<ircode::Variable>& var1, const std::shared_ptr<ircode::Variable>& var2) {
            return var1->getName(true) < var2->getName(true);
        });
        for (auto var : varList) {
            ss << var->getName(true) << std::endl;
        }
        return Compare(ss.str(), expectedCode);
    }
};

TEST_F(SimilarityResearcherTest, Test) {
    /*
        void main() {
            func1(globalVar_0x100);
            globalVar_0x300 = func2();
        }

        void func1(int param1) {
            globalVar_0x200 = param1 + 1;
        }

        int func2() {
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
    auto expectedDataFlowOfMain = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x100 \n\
        var3 <- Read var2 \n\
        var4 <- Copy var3 \n\
        var6 <- Copy Bb:var5 \n\
        var7 <- Copy var1 + 0x300 \n\
        var8 <- Write var7 \n\
        var8 <- Write var6 \n\
        B7:var3 <- Copy var4 \
    ";
    auto expectedDataFlowOfFunc1 = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x200 \n\
        var3 <- Copy B0:var4 \n\
        var4 <- Unknown \n\
        var5 <- Write var2 \n\
        var5 <- Write var4 \
    ";
    auto expectedDataFlowOfFunc2 = "\
        var1 <- Copy Start \n\
        var2 <- Copy var1 + 0x200 \n\
        var3 <- Read var2 \n\
        var4 <- Copy var3 \n\
        var5 <- Unknown \n\
        B0:var6 <- Copy var5 \
    ";
    auto expectedStructures = "\
        struct root { \n\
            0x100: B0:var3 \n\
            0x200: B7:var4, Bb:var3 \n\
            0x300: Bb:var5 \n\
        } \
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
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMain));
    ASSERT_TRUE(cmp(func1, expectedIRCodeOfFunc1));
    ASSERT_TRUE(cmp(func2, expectedIRCodeOfFunc2));
    ASSERT_TRUE(cmpDataFlow(mainFunction, expectedDataFlowOfMain));
    ASSERT_TRUE(cmpDataFlow(func1, expectedDataFlowOfFunc1));
    ASSERT_TRUE(cmpDataFlow(func2, expectedDataFlowOfFunc2));
    ASSERT_TRUE(cmpStructures(expectedStructures));

    auto expectedVariables = "\
        B0:var3 \n\
        B0:var4 \n\
        B0:var6 \n\
        B7:var3 \n\
        B7:var4 \n\
        Bb:var3 \n\
        Bb:var4 \n\
        Bb:var5 \
    ";
    simResearcher->research(func2->findVariableById(5));
    ASSERT_TRUE(cmpVariables(simResearcher->getVariables(), expectedVariables));
}
