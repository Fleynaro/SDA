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

    void SetUp() override {
        IRcodeFixture::SetUp();
        auto globalVarRepo = std::make_unique<semantics::GlobalVarSemanticsRepository>(&semManager, context->getPlatform());
        this->globalVarRepo = globalVarRepo.get();
        semManager.addRepository(std::move(globalVarRepo));
    }
};

TEST_F(SemanticsTest, Simplest) {
    auto sourcePCode = "\
        r10:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE r10:8, 0x0:8 \n\
        rax:8 = LOAD r10:8, 8:8 \
    ";
    auto expectedIRCode = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3[var1 + 16]:8 = COPY 0x0:8 \n\
            var4[rax]:8 = COPY var3 \
    ";
    auto function = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(function, expectedIRCode));
    auto semList = globalVarRepo->findSemanticsAtOffset(0x0);
    ASSERT_EQ(semList.size(), 1);
}
