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
    std::unique_ptr<ircode::ContextSync> ctxSync;

    void SetUp() override {
        SemanticsFixture::SetUp();
        auto callConv = std::make_shared<platform::FastcallCallingConvention>();

        ctxSync = std::make_unique<ircode::ContextSync>(globalSymbolTable, callConv);
        program->getEventPipe()->connect(ctxSync->getEventPipe());

        signatureRepo = std::make_unique<semantics::SignatureRepository>();
        signatureCollector = std::make_unique<semantics::SignatureCollector>(
            program,
            context->getPlatform(),
            callConv,
            signatureRepo.get());
    }
};

TEST_F(SignatureSemanticsTest, Simple1) {
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
        signature fastcall void () \
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

TEST_F(SignatureSemanticsTest, Simple2) {
    auto sourcePCode = "\
        // main() \n\
        CALL <getValue> \n\
        RETURN \n\
        \n\
        \n\
        // uint32_t getValue() \n\
        <getValue>: \n\
        rax:4 = COPY 1000:4 \n\
        RETURN \n\
    ";
    auto expectedIRCodeOfMainFunc = "\
        Block B0(level: 1): \n\
            var1[rax]:4 = CALL 0x200:8 \
    ";
    auto expectedSigOfMainFunc = "\
        signature fastcall uint32_t () \
    ";
    auto expectedIRCodeOfFunc2 = "\
        Block B2(level: 1): \n\
            var1[rax]:4 = COPY 0x3e8:4 \
    ";
    auto expectedSigOfFunc2 = "\
        signature fastcall uint32_t () \
    ";
    auto mainFunction = parsePcode(sourcePCode, program);
    auto func2 = program->toFunction(
        graph->getFunctionGraphAt(pcode::InstructionOffset(2, 0)));
    ASSERT_TRUE(cmp(mainFunction, expectedIRCodeOfMainFunc));
    ASSERT_TRUE(cmp(func2, expectedIRCodeOfFunc2));
    ASSERT_TRUE(cmpDataType(mainFunction->getFunctionSymbol()->getSignature(), expectedSigOfMainFunc));
    ASSERT_TRUE(cmpDataType(func2->getFunctionSymbol()->getSignature(), expectedSigOfFunc2));
}
