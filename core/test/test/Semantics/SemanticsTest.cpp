#include "Test/Core/IRcode/IRcodeFixture.h"
#include "Test/Core/Utils/TestAssertion.h"
#include "SDA/Core/Semantics/Semantics.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class SemanticsTest : public IRcodeFixture
{
protected:
    semantics::SemanticsManager semManager = semantics::SemanticsManager(&program);
    semantics::GlobalVarSemanticsRepository* globalVarRepo = nullptr;
    semantics::SymbolTableSemanticsRepository* symbolTableRepo = nullptr;

    void addGlobalVarSemanticsRepository() {
        auto globalVarRepo = std::make_unique<semantics::GlobalVarSemanticsRepository>(&semManager, context->getPlatform());
        this->globalVarRepo = globalVarRepo.get();
        semManager.addRepository(std::move(globalVarRepo));
    }

    void addSymbolTableSemanticsRepository() {
        auto symbolTableRepo = std::make_unique<semantics::SymbolTableSemanticsRepository>(&semManager, globalSymbolTable);
        this->symbolTableRepo = symbolTableRepo.get();
        semManager.addRepository(std::move(symbolTableRepo));
    }
};

TEST_F(SemanticsTest, Simplest) {
    /*
        float setGlobalVar(float value) {
            globalVar = value;
            return globalVar;
        }
    */
   addSymbolTableSemanticsRepository();
    auto func = newFunction(
        0,
        "main",
        "sig = signature fastcall float(float value)");

    auto sourcePCode = "\
        r10:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE r10:8, xmm0:Da \n\
        rax:8 = LOAD r10:8, 4:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm0 \n\
            var4[var1 + 16]:4 = COPY var3 \n\
            var5:8 = LOAD var2 \n\
            var6[rax]:8 = COPY var5 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    auto semList = globalVarRepo->findSemanticsAt(0x0);
    ASSERT_EQ(semList.size(), 1);
}
