#include "Test/Core/Semantics/SemanticsFixture.h"
#include "SDA/Core/Semantics/Signature.h"
#include "SDA/Platform/X86/CallingConvention.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class SignatureSemanticsTest : public SemanticsFixture
{
protected:
    std::unique_ptr<semantics::SignatureRepository> signatureRepo;
    std::unique_ptr<semantics::SignatureCollector> signatureCollector;

    void SetUp() override {
        IRcodeFixture::SetUp();
        // signatureRepo = std::make_unique<semantics::SignatureRepository>();
        // signatureCollector = std::make_unique<semantics::SignatureCollector>(
        //     &program,
        //     context->getPlatform(),
        //     std::make_shared<platform::FastcallCallingConvention>(),
        //     signatureRepo.get());
    }
};

TEST_F(SignatureSemanticsTest, Simple) {
    auto sourcePCode = "\
        // main() \n\
        xmm0:Da = COPY 0.5:4 \n\
        CALL <setGlobalFloatValue> \n\
        RETURN \n\
        \n\
        \n\
        // setGlobalFloatValue(float value)\n\
        <setGlobalFloatValue>: \n\
        $0:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE $0:8, xmm0:Da \n\
        RETURN \n\
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[r10]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm0 \n\
            var4[var2]:4 = COPY var3 \n\
            var5:8 = LOAD var2 \n\
            var6[rax]:8 = COPY var5 \
    ";
    auto mainFunction = parsePcode(sourcePCode, &program);
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
}
