#include "SDA/Platform/X86/PcodeDecoder.h"
#include "SDA/Platform/X86/RegisterRepository.h"
#include "SDA/Core/Pcode/PcodePrinter.h"
#include "Test/Core/Utils/TestAssertion.h"
#include <gtest/gtest.h>

using namespace sda;
using namespace sda::test;
using namespace sda::platform;

std::shared_ptr<PcodeDecoder> GetPcodeDecoder() {
    auto decoder = std::make_unique<ZydisDecoder>();
    ZydisDecoderInit(decoder.get(), ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    return std::make_shared<PcodeDecoderX86>(std::move(decoder));
}

std::list<pcode::Instruction> Decode(const std::vector<uint8_t>& asmCode) {
    std::list<pcode::Instruction> result;
    auto decoder = GetPcodeDecoder();
    size_t offset = 0;
    while (offset < asmCode.size()) {
        std::vector<uint8_t> data;
        data.insert(data.end(), asmCode.begin() + offset, asmCode.end());
        decoder->decode(offset, data);
        auto instructions = decoder->getDecodedInstructions();
        result.insert(result.end(), instructions.begin(), instructions.end());
        offset += decoder->getInstructionLength();
    }
    return result;
}

::testing::AssertionResult CmpPcode(const std::list<pcode::Instruction>& instructions, const std::string& expectedCode) {
    RegisterRepositoryX86 regRepo;
    pcode::Printer pcodePrinter(&regRepo);
    std::stringstream ss;
    pcodePrinter.setOutput(ss);
    for (size_t i = 0; i < 2; ++i)
        pcodePrinter.startBlock();
    pcodePrinter.newTabs();
    for (auto instruction : instructions) {
        pcodePrinter.printInstruction(&instruction);
        pcodePrinter.newLine();
    }
    return Compare(ss.str(), expectedCode);
}

TEST(PcodeDecoderX86Test, test1) {
    /*
        mov eax, 0x1
        mov ebx, 0x2
        add eax, ebx
    */
    std::vector<uint8_t> asmCode = {
        0xB8, 0x01, 0x00, 0x00, 0x00,
        0xBB, 0x02, 0x00, 0x00, 0x00,
        0x01, 0xD8
    };
    auto expectedCode = "\
        rax:8 = COPY 0x1:4 \n\
        rbx:8 = COPY 0x2:4 \n\
        CF:1 = INT_CARRY rax:4, rbx:4 \n\
        OF:1:11 = INT_SCARRY rax:4, rbx:4 \n\
        rax:8 = INT_ADD rax:4, rbx:4 \n\
        SF:1:7 = INT_SLESS rax:4, 0x0:4 \n\
        ZF:1:6 = INT_EQUAL rax:4, 0x0:4 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}