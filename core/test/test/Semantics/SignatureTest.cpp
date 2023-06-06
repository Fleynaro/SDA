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
        SemanticsFixture::SetUp();
        auto callConv = std::make_shared<platform::FastcallCallingConvention>();

        auto programCallbacks = std::make_shared<ircode::ContextSyncCallbacks>(globalSymbolTable, callConv);
        auto prevCallbacks = program->getCallbacks();
        programCallbacks->setPrevCallbacks(prevCallbacks);
        program->setCallbacks(programCallbacks);

        signatureRepo = std::make_unique<semantics::SignatureRepository>();
        signatureCollector = std::make_unique<semantics::SignatureCollector>(
            program,
            context->getPlatform(),
            callConv,
            signatureRepo.get());
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
        // setGlobalFloatValue(float value) \n\
        <setGlobalFloatValue>: \n\
        $0:8 = INT_ADD rip:8, 0x10:8 \n\
        STORE $0:8, xmm0:Da \n\
        RETURN \n\
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1): \n\
            var1[xmm0]:4 = COPY 0x0:4 \n\
            var2:1 = CALL 0x300:8, var1 \
    ";
    auto expectedSigOfMainFunc = "\
        signature fastcall float () \
    ";
    auto expectedIRCodeOfFunc2 = "\
        Block B3(level: 1): \n\
            var1:8 = LOAD rip \n\
            var2[$U0]:8 = INT_ADD var1, 0x10:8 \n\
            var3:4 = LOAD xmm0 \n\
            var4[var2]:4 = COPY var3 \
    ";
    auto expectedSigOfFunc2 = "\
        signature fastcall void (float param1) \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto func2 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(3, 0)));
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmp(func2, expectedIRCodeOfFunc2));
    ASSERT_TRUE(cmpDataType(mainFunction->getFunctionSymbol()->getSignature(), expectedSigOfMainFunc));
    ASSERT_TRUE(cmpDataType(func2->getFunctionSymbol()->getSignature(), expectedSigOfFunc2));
}
