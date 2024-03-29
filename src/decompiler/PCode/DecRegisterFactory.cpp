#include "DecRegisterFactory.h"

using namespace CE::Decompiler;

Register RegisterFactoryX86::createRegister(int regId, int size, int64_t offset) {
	return CreateRegister(static_cast<ZydisRegister>(regId), size, offset);
}

Register RegisterFactoryX86::createFlagRegister(int flag) {
	return CreateFlagRegister(static_cast<ZydisCPUFlag>(flag));
}

Register RegisterFactoryX86::createInstructionPointerRegister() {
	return createRegister(ZYDIS_REGISTER_RIP, 0x8);
}

Register RegisterFactoryX86::createStackPointerRegister() {
	return createRegister(ZYDIS_REGISTER_RSP, 0x8);
}

Register RegisterFactoryX86::CreateRegister(ZydisRegister reg, int size, int64_t offset) {
	const auto mask = BitMask64(size) << static_cast<int>((offset % 8) * 8);
	const auto index = static_cast<int>(offset) / 8;

	if (reg == ZYDIS_REGISTER_RIP)
		return Register(reg, 0, mask, Register::Type::InstructionPointer);
	if (reg == ZYDIS_REGISTER_RSP)
		return Register(reg, 0, mask, Register::Type::StackPointer);

	if (reg >= ZYDIS_REGISTER_AL && reg <= ZYDIS_REGISTER_BL) {
		return Register(ZYDIS_REGISTER_RAX + reg - ZYDIS_REGISTER_AL, 0, mask);
	}
	if (reg >= ZYDIS_REGISTER_AH && reg <= ZYDIS_REGISTER_BH) {
		return Register(ZYDIS_REGISTER_RAX + reg - ZYDIS_REGISTER_AH, 0, mask);
	}
	if (reg >= ZYDIS_REGISTER_SPL && reg <= ZYDIS_REGISTER_R15B) {
		return Register(ZYDIS_REGISTER_RAX + reg - ZYDIS_REGISTER_AH, 0, mask);
	}
	if (reg >= ZYDIS_REGISTER_AX && reg <= ZYDIS_REGISTER_R15W) {
		return Register(ZYDIS_REGISTER_RAX + reg - ZYDIS_REGISTER_AX, 0, mask);
	}
	if (reg >= ZYDIS_REGISTER_EAX && reg <= ZYDIS_REGISTER_R15D) {
		return Register(ZYDIS_REGISTER_RAX + reg - ZYDIS_REGISTER_EAX, 0, mask);
	}
	if (reg >= ZYDIS_REGISTER_RAX && reg <= ZYDIS_REGISTER_R15) {
		return Register(reg, 0, mask);
	}
	if (reg >= ZYDIS_REGISTER_MM0 && reg <= ZYDIS_REGISTER_MM7) {
		return Register(reg, 0, mask, Register::Type::Vector);
	}
	if (reg >= ZYDIS_REGISTER_XMM0 && reg <= ZYDIS_REGISTER_XMM31) {
		return Register(reg, index, mask, Register::Type::Vector);
	}
	// todo: ymm/zmm support

	return Register();
}

Register RegisterFactoryX86::CreateFlagRegister(ZydisCPUFlag flag) {
	const BitMask64 mask = static_cast<uint64_t>(1) << flag;
	return Register(ZYDIS_REGISTER_RFLAGS, 0, mask, Register::Type::Flag);
}
