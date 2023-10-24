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
            dataFlowRepo.get());
        semResearcher->addPropagator(
            std::make_unique<researcher::BaseSemanticsPropagator>(semRepo.get()));
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
            for (auto& semantics : obj.getSemantics()) {
                semList.push_back(semantics->toString());
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
    auto expectedDataFlowOfFunc = "\
        var1 <- Unknown \n\
        var2 <- Copy var1 \n\
        var3 <- Unknown \n\
        var4 <- Copy var3 \
    ";
    auto expectedSemantics = "";
    auto func = parsePcode(sourcePCode, program);
    auto funcSigDt = dynamic_cast<SignatureDataType*>(
        parseDataType(funcSig));
    func->getFunctionSymbol()->getSignature()->copyFrom(funcSigDt);
    ASSERT_TRUE(cmp(func, expectedIRCodeOfFunc));
    ASSERT_TRUE(cmpDataFlow(func, expectedDataFlowOfFunc));
    ASSERT_TRUE(cmpSemantics(expectedSemantics));
}
