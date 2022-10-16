#include "SDA/Platform/X86/RegisterRepository.h"
#include <map>
#include <stdexcept>
#include <Zydis/Zydis.h>

using namespace sda::platform;

std::map<std::string, ZydisRegister> RegisterNameToId = {
    // General purpose registers 64-bit
    {"rax", ZYDIS_REGISTER_RAX},
    {"rcx", ZYDIS_REGISTER_RCX},
    {"rdx", ZYDIS_REGISTER_RDX},
    {"rbx", ZYDIS_REGISTER_RBX},
    {"rsp", ZYDIS_REGISTER_RSP},
    {"rbp", ZYDIS_REGISTER_RBP},
    {"rsi", ZYDIS_REGISTER_RSI},
    {"rdi", ZYDIS_REGISTER_RDI},
    {"r8", ZYDIS_REGISTER_R8},
    {"r9", ZYDIS_REGISTER_R9},
    {"r10", ZYDIS_REGISTER_R10},
    {"r11", ZYDIS_REGISTER_R11},
    {"r12", ZYDIS_REGISTER_R12},
    {"r13", ZYDIS_REGISTER_R13},
    {"r14", ZYDIS_REGISTER_R14},
    {"r15", ZYDIS_REGISTER_R15},

    // Floating point multimedia registers
    {"mm0", ZYDIS_REGISTER_MM0},
    {"mm1", ZYDIS_REGISTER_MM1},
    {"mm2", ZYDIS_REGISTER_MM2},
    {"mm3", ZYDIS_REGISTER_MM3},
    {"mm4", ZYDIS_REGISTER_MM4},
    {"mm5", ZYDIS_REGISTER_MM5},
    {"mm6", ZYDIS_REGISTER_MM6},
    {"mm7", ZYDIS_REGISTER_MM7},
    
    // Floating point vector registers 128-bit
    {"xmm0", ZYDIS_REGISTER_XMM0},
    {"xmm1", ZYDIS_REGISTER_XMM1},
    {"xmm2", ZYDIS_REGISTER_XMM2},
    {"xmm3", ZYDIS_REGISTER_XMM3},
    {"xmm4", ZYDIS_REGISTER_XMM4},
    {"xmm5", ZYDIS_REGISTER_XMM5},
    {"xmm6", ZYDIS_REGISTER_XMM6},
    {"xmm7", ZYDIS_REGISTER_XMM7},
    {"xmm8", ZYDIS_REGISTER_XMM8},
    {"xmm9", ZYDIS_REGISTER_XMM9},
    {"xmm10", ZYDIS_REGISTER_XMM10},
    {"xmm11", ZYDIS_REGISTER_XMM11},
    {"xmm12", ZYDIS_REGISTER_XMM12},
    {"xmm13", ZYDIS_REGISTER_XMM13},
    {"xmm14", ZYDIS_REGISTER_XMM14},
    {"xmm15", ZYDIS_REGISTER_XMM15},
    {"xmm16", ZYDIS_REGISTER_XMM16},
    {"xmm17", ZYDIS_REGISTER_XMM17},
    {"xmm18", ZYDIS_REGISTER_XMM18},
    {"xmm19", ZYDIS_REGISTER_XMM19},
    {"xmm20", ZYDIS_REGISTER_XMM20},
    {"xmm21", ZYDIS_REGISTER_XMM21},
    {"xmm22", ZYDIS_REGISTER_XMM22},
    {"xmm23", ZYDIS_REGISTER_XMM23},
    {"xmm24", ZYDIS_REGISTER_XMM24},
    {"xmm25", ZYDIS_REGISTER_XMM25},
    {"xmm26", ZYDIS_REGISTER_XMM26},
    {"xmm27", ZYDIS_REGISTER_XMM27},
    {"xmm28", ZYDIS_REGISTER_XMM28},
    {"xmm29", ZYDIS_REGISTER_XMM29},
    {"xmm30", ZYDIS_REGISTER_XMM30},
    {"xmm31", ZYDIS_REGISTER_XMM31},
};

std::string RegisterRepositoryX86::getRegisterName(size_t regId) const {
    return ZydisRegisterGetString(static_cast<ZydisRegister>(regId));
}

size_t RegisterRepositoryX86::getRegisterId(const std::string& regName) const {
    return RegisterNameToId.at(regName);
}

sda::Register::Type RegisterRepositoryX86::getRegisterType(size_t regId) const {
    if (regId == ZYDIS_REGISTER_RIP)
        return Register::InstructionPointer;
    if (regId == ZYDIS_REGISTER_RSP)
        return Register::StackPointer;
	if (regId >= ZYDIS_REGISTER_MM0 && regId <= ZYDIS_REGISTER_MM7 ||
        regId >= ZYDIS_REGISTER_XMM0 && regId <= ZYDIS_REGISTER_XMM31)
		return Register::Vector;
	return Register::Generic;
}

std::string RegisterRepositoryX86::getRegisterFlagName(size_t flagMask) const {
    std::string flagName;
    if ((flagMask & ZYDIS_CPUFLAG_CF) != 0)
        flagName = "CF";
    else if ((flagMask & ZYDIS_CPUFLAG_OF) != 0)
        flagName = "OF";
    else if ((flagMask & ZYDIS_CPUFLAG_SF) != 0)
        flagName = "SF";
    else if ((flagMask & ZYDIS_CPUFLAG_ZF) != 0)
        flagName = "ZF";
    else if ((flagMask & ZYDIS_CPUFLAG_AF) != 0)
        flagName = "AF";
    else if ((flagMask & ZYDIS_CPUFLAG_PF) != 0)
        flagName = "PF";
    throw std::runtime_error("Unknown flag mask");
}

size_t RegisterRepositoryX86::getRegisterFlagIndex(const std::string& flagName) const {
    if (flagName == "CF")
        return ZYDIS_CPUFLAG_CF;
    else if (flagName == "OF")
        return ZYDIS_CPUFLAG_OF;
    else if (flagName == "SF")
        return ZYDIS_CPUFLAG_SF;
    else if (flagName == "ZF")
        return ZYDIS_CPUFLAG_ZF;
    else if (flagName == "AF")
        return ZYDIS_CPUFLAG_AF;
    else if (flagName == "PF")
        return ZYDIS_CPUFLAG_PF;
    throw std::runtime_error("Unknown flag name");
}

size_t RegisterRepositoryX86::transformZydisRegId(ZydisRegister regId) const {
    if (regId == ZYDIS_REGISTER_RIP) {
		return Register::InstructionPointerId;
    }
    else if (regId == ZYDIS_REGISTER_RSP) {
		return Register::StackPointerId;
    }
	else if (regId >= ZYDIS_REGISTER_AL && regId <= ZYDIS_REGISTER_BL) {
		return ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_AL;
	}
	else if (regId >= ZYDIS_REGISTER_AH && regId <= ZYDIS_REGISTER_BH) {
		return ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_AH;
	}
	else if (regId >= ZYDIS_REGISTER_SPL && regId <= ZYDIS_REGISTER_R15B) {
		return ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_AH;
	}
	else if (regId >= ZYDIS_REGISTER_AX && regId <= ZYDIS_REGISTER_R15W) {
		return ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_AX;
	}
	else if (regId >= ZYDIS_REGISTER_EAX && regId <= ZYDIS_REGISTER_R15D) {
		return ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_EAX;
	}
	else if (regId >= ZYDIS_REGISTER_RAX && regId <= ZYDIS_REGISTER_R15) {
		return regId;
	}
	else if (regId >= ZYDIS_REGISTER_MM0 && regId <= ZYDIS_REGISTER_MM7) {
		return regId;
	}
	else if (regId >= ZYDIS_REGISTER_XMM0 && regId <= ZYDIS_REGISTER_XMM31) {
		return regId;
	}
    throw std::runtime_error("Unknown register");
}