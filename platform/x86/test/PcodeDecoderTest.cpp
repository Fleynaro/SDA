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

TEST(PcodeDecoderX86Test, addTwoRegisters) {
    /*
        Asm:
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
        CF = INT_CARRY rax:4, rbx:4 \n\
        OF = INT_SCARRY rax:4, rbx:4 \n\
        rax:8 = INT_ADD rax:4, rbx:4 \n\
        SF = INT_SLESS rax:4, 0x0:4 \n\
        ZF = INT_EQUAL rax:4, 0x0:4 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, constToEax) {
    /*
        That is the feature of x86: setting value to EAX cleans fully RAX

        Asm:
        mov eax, 0x1
    */
    std::vector<uint8_t> asmCode = {
        0xB8, 0x01, 0x00, 0x00, 0x00
    };
    auto expectedCode = "\
        rax:8 = COPY 0x1:4 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, push_pop) {
    /*
        Asm:
        push rax
        pop rbx
    */
    std::vector<uint8_t> asmCode = {
        0x50,
        0x5B
    };
    auto expectedCode = "\
        rsp:8 = INT_SUB rsp:8, 0x8:8 \n\
        STORE rsp:8, rax:8 \n\
        rbx:8 = LOAD rsp:8 \n\
        rsp:8 = INT_ADD rsp:8, 0x8:8 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, test_je) {
    /*
        Asm:
        test rax,rax
        je 0x10
    */
    std::vector<uint8_t> asmCode = {
        0x48, 0x85, 0xC0,
        0x0F, 0x84, 0x00, 0x00, 0x00, 0x00
    };
    auto expectedCode = "\
        CF = COPY 0x0:8 \n\
        OF = COPY 0x0:8 \n\
        $U1:8 = INT_AND rax:8, rax:8 \n\
        SF = INT_SLESS $U1:8, 0x0:8 \n\
        ZF = INT_EQUAL $U1:8, 0x0:8 \n\
        CBRANCH 0x900:8, ZF \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, ret) {
    /*
        Asm:
        ret
    */
    std::vector<uint8_t> asmCode = {
        0xC3
    };
    auto expectedCode = "\
        rip:8 = LOAD rsp:8 \n\
        rsp:8 = INT_ADD rsp:8, 0x8:8 \n\
        RETURN rip:8 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, movaps) {
    /*
        Asm:
        movaps xmm1,xmm2
    */
    std::vector<uint8_t> asmCode = {
        0x0F, 0x28, 0xCA
    };
    auto expectedCode = "\
        xmm1:Da = COPY xmm2:Da \n\
        xmm1:Db = COPY xmm2:Db \n\
        xmm1:Dc = COPY xmm2:Dc \n\
        xmm1:Dd = COPY xmm2:Dd \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, movapd) {
    /*
        Asm:
        movapd xmm1,xmm2
    */
    std::vector<uint8_t> asmCode = {
        0x66, 0x0F, 0x28, 0xCA
    };
    auto expectedCode = "\
        xmm1:Qa = COPY xmm2:Qa \n\
        xmm1:Qb = COPY xmm2:Qb \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, movups1) {
    /*
        Asm:
        movups xmm0,XMMWORD PTR [rsp+0x20]
    */
    std::vector<uint8_t> asmCode = {
        0x0F, 0x10, 0x44, 0x24, 0x20
    };
    auto expectedCode = "\
        $U1:8 = INT_ADD rsp:8, 0x20:8 \n\
        $U2:4 = LOAD $U1:8 \n\
        xmm0:Da = COPY $U2:4 \n\
        $U3:8 = INT_ADD rsp:8, 0x20:8 \n\
        $U4:8 = INT_ADD $U3:8, 0x4:8 \n\
        $U5:4 = LOAD $U4:8 \n\
        xmm0:Db = COPY $U5:4 \n\
        $U6:8 = INT_ADD rsp:8, 0x20:8 \n\
        $U7:8 = INT_ADD $U6:8, 0x8:8 \n\
        $U8:4 = LOAD $U7:8 \n\
        xmm0:Dc = COPY $U8:4 \n\
        $U9:8 = INT_ADD rsp:8, 0x20:8 \n\
        $U10:8 = INT_ADD $U9:8, 0xc:8 \n\
        $U11:4 = LOAD $U10:8 \n\
        xmm0:Dd = COPY $U11:4 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, movups2) {
    /*
        Asm:
        movups XMMWORD PTR [rsp+0x20],xmm0
    */
    std::vector<uint8_t> asmCode = {
        0x0F, 0x11, 0x44, 0x24, 0x20
    };
    auto expectedCode = "\
        $U1:4 = COPY xmm0:Da \n\
        $U2:8 = INT_ADD rsp:8, 0x20:8 \n\
        STORE $U2:8, $U1:4 \n\
        $U3:4 = COPY xmm0:Db \n\
        $U4:8 = INT_ADD rsp:8, 0x20:8 \n\
        $U5:8 = INT_ADD $U4:8, 0x4:8 \n\
        STORE $U5:8, $U3:4 \n\
        $U6:4 = COPY xmm0:Dc \n\
        $U7:8 = INT_ADD rsp:8, 0x20:8 \n\
        $U8:8 = INT_ADD $U7:8, 0x8:8 \n\
        STORE $U8:8, $U6:4 \n\
        $U9:4 = COPY xmm0:Dd \n\
        $U10:8 = INT_ADD rsp:8, 0x20:8 \n\
        $U11:8 = INT_ADD $U10:8, 0xc:8 \n\
        STORE $U11:8, $U9:4 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, movss) {
    /*
        Asm:
        movss  DWORD PTR [rax],xmm1
    */
    std::vector<uint8_t> asmCode = {
        0xF3, 0x0F, 0x11, 0x08
    };
    auto expectedCode = "\
        $U1:4 = COPY xmm1:Da \n\
        STORE rax:8, $U1:4 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, movsd) {
    /*
        Asm:
        movsd xmm1,QWORD PTR [rax]
    */
    std::vector<uint8_t> asmCode = {
        0xF2, 0x0F, 0x11, 0x08
    };
    auto expectedCode = "\
        $U1:8 = COPY xmm1:Qa \n\
        STORE rax:8, $U1:8 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, lea) {
    /*
        Asm:
        lea rax,[rcx+0x10]
    */
    std::vector<uint8_t> asmCode = {
        0x48, 0x8D, 0x41, 0x10
    };
    auto expectedCode = "\
        $U1:8 = INT_ADD rcx:8, 0x10:8 \n\
        rax:8 = COPY $U1:8 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, shufps55) {
    /*
        0x55 = 0b(01)(01)(01)(01) = (1)(1)(1)(1)

        Asm:
        shufps xmm1,xmm2,0x55
    */
    std::vector<uint8_t> asmCode = {
        0x0F, 0xC6, 0xCA, 0x55
    };
    auto expectedCode = "\
        xmm1:Da = COPY xmm1:Db \n\
        xmm1:Db = COPY xmm1:Db \n\
        xmm1:Dc = COPY xmm2:Db \n\
        xmm1:Dd = COPY xmm2:Db \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, shufpsE4) {
    /*
        0xE4 = 0b(11)(10)(01)(00) = (3)(2)(1)(0)

        Asm:
        shufps xmm1,xmm2,0xE4
    */
    std::vector<uint8_t> asmCode = {
        0x0F, 0xC6, 0xCA, 0xE4
    };
    auto expectedCode = "\
        xmm1:Da = COPY xmm1:Da \n\
        xmm1:Db = COPY xmm1:Db \n\
        xmm1:Dc = COPY xmm2:Dc \n\
        xmm1:Dd = COPY xmm2:Dd \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, cvtdq2pd) {
    /*
        Asm:
        cvtdq2pd xmm1,xmm1
    */
    std::vector<uint8_t> asmCode = {
        0xF3, 0x0F, 0xE6, 0xC9
    };
    auto expectedCode = "\
        xmm1:Qa = INT2FLOAT xmm1:Da \n\
        xmm1:Qb = INT2FLOAT xmm1:Dc \n\
        xmm1:Qc = COPY 0x0:8 \n\
        xmm1:Qc = COPY 0x0:8 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, cvtpd2dq) {
    /*
        Asm:
        cvtpd2dq xmm1,xmm1
    */
    std::vector<uint8_t> asmCode = {
        0xF2, 0x0F, 0xE6, 0xC9
    };
    auto expectedCode = "\
        xmm1:Da = FLOAT2INT xmm1:Qa \n\
        xmm1:Dc = FLOAT2INT xmm1:Qb \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, mulss) {
    /*
        Asm:
        mulss xmm1,xmm1
    */
    std::vector<uint8_t> asmCode = {
        0xF3, 0x0F, 0x59, 0xC9
    };
    auto expectedCode = "\
        xmm1:Da = FLOAT_MULT xmm1:Da, xmm1:Da \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, xorps) {
    /*
        Asm:
        xorps xmm1,xmm1
    */
    std::vector<uint8_t> asmCode = {
        0x0F, 0x57, 0xC9
    };
    auto expectedCode = "\
        xmm1:Da = INT_XOR xmm1:Da, xmm1:Da \n\
        xmm1:Db = INT_XOR xmm1:Db, xmm1:Db \n\
        xmm1:Dc = INT_XOR xmm1:Dc, xmm1:Dc \n\
        xmm1:Dd = INT_XOR xmm1:Dd, xmm1:Dd \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, comiss_jbe) {
    /*
        bool cmp(float param1, float param2) {
            return isNan(param1) || isNan(param2) || param1 == param2 || param1 < param2;
        }

        Asm:
        comiss xmm1,xmm2
        jbe 0x10
    */
    std::vector<uint8_t> asmCode = {
        0x0F, 0x2F, 0xCA,
        0x0F, 0x86, 0x00, 0x00, 0x00, 0x00
    };
    auto expectedCode = "\
        $U1:1 = FLOAT_NAN xmm1:Da \n\
        $U2:1 = FLOAT_NAN xmm2:Da \n\
        PF = BOOL_OR $U1:1, $U2:1 \n\
        $U3:1 = FLOAT_EQUAL xmm1:Da, xmm2:Da \n\
        ZF = BOOL_OR PF, $U3:1 \n\
        $U4:1 = FLOAT_LESS xmm1:Da, xmm2:Da \n\
        CF = BOOL_OR PF, $U4:1 \n\
        OF = COPY 0x0:1 \n\
        AF = COPY 0x0:1 \n\
        SF = COPY 0x0:1 \n\
        $U1:1 = BOOL_OR CF, ZF \n\
        CBRANCH 0x900:8, $U1:1 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, comiss_jae) {
    /*
        bool cmp(float param1, float param2) {
            return !isNan(param1) && !isNan(param2) && param1 >= param2;
        }

        Asm:
        comiss xmm1,xmm2
        jae 0x10
    */
    std::vector<uint8_t> asmCode = {
        0x0F, 0x2F, 0xCA,
        0x0F, 0x83, 0x00, 0x00, 0x00, 0x00
    };
    auto expectedCode = "\
        $U1:1 = FLOAT_NAN xmm1:Da \n\
        $U2:1 = FLOAT_NAN xmm2:Da \n\
        PF = BOOL_OR $U1:1, $U2:1 \n\
        $U3:1 = FLOAT_EQUAL xmm1:Da, xmm2:Da \n\
        ZF = BOOL_OR PF, $U3:1 \n\
        $U4:1 = FLOAT_LESS xmm1:Da, xmm2:Da \n\
        CF = BOOL_OR PF, $U4:1 \n\
        OF = COPY 0x0:1 \n\
        AF = COPY 0x0:1 \n\
        SF = COPY 0x0:1 \n\
        $U1:1 = BOOL_NEGATE CF \n\
        CBRANCH 0x900:8, $U1:1 \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}

TEST(PcodeDecoderX86Test, sqrtss) {
    /*
        Asm:
        sqrtss xmm1,xmm2
    */
    std::vector<uint8_t> asmCode = {
        0xF3, 0x0F, 0x51, 0xCA
    };
    auto expectedCode = "\
        xmm1:Da = FLOAT_SQRT xmm2:Da \
    ";
    auto instructions = Decode(asmCode);
    ASSERT_TRUE(CmpPcode(instructions, expectedCode));
}
