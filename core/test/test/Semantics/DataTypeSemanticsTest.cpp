#include "Test/Core/Semantics/SemanticsFixture.h"
#include "SDA/Core/Semantics/DataTypeSemantics.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class DataTypeSemanticsTest : public SemanticsFixture
{
protected:
    semantics::SymbolTableSemanticsRepository* symbolTableRepo = nullptr;
    semantics::DataTypeSemanticsRepository* dataTypeRepo = nullptr;

    void SetUp() override {
        SemanticsFixture::SetUp();
        // add repositories
        auto symbolTableRepo = std::make_unique<semantics::SymbolTableSemanticsRepository>(semManager);
        auto dataTypeRepo = std::make_unique<semantics::DataTypeSemanticsRepository>(semManager);
        this->symbolTableRepo = symbolTableRepo.get();
        this->dataTypeRepo = dataTypeRepo.get();
        semManager->addRepository(std::move(symbolTableRepo));
        semManager->addRepository(std::move(dataTypeRepo));
        semManager->addPropagator(
            std::make_unique<semantics::DataTypeSemanticsPropagator>(globalSymbolTable, this->symbolTableRepo, this->dataTypeRepo));
    }
};

TEST_F(DataTypeSemanticsTest, Simplest) {
    /*
        float setGlobalVar(float value) {
            globalVar = value;
            return globalVar;
        }
    */

    newFunction(
        0,
        "main",
        "sig = signature fastcall float(float value)");

    auto sourcePCode = "\
        r10:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE r10:8, xmm0:Da \n\
        rax:8 = LOAD r10:8, 4:8 \
    ";
    // TODO: fix this
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm0 \n\
            var4[var1 + 16]:4 = COPY var3 \n\
            var5:8 = LOAD var2 \n\
            var6[rax]:8 = COPY var5 \
    ";
    //auto function = parsePcode(sourcePCode, &program);
    //ASSERT_TRUE(cmp(function, expectedIRCode));
    //auto semList = simRepo->findSemanticsAt(0x0);
    //ASSERT_EQ(semList.size(), 1);
}
