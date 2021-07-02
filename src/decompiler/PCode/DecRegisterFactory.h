#pragma once
#include "DecPCode.h"

namespace CE::Decompiler
{
	using namespace PCode;

	// register factory which have to be defined for each architecture
	class AbstractRegisterFactory
	{
	public:
		// normal registers
		virtual Register createRegister(int regId, int size, int64_t offset = 0x0) = 0;

		// flag registers (need for condition)
		virtual Register createFlagRegister(int flag) = 0;

		// IP
		virtual Register createInstructionPointerRegister() = 0;

		// SP
		virtual Register createStackPointerRegister() = 0;
	};

	class RegisterFactoryX86 : public AbstractRegisterFactory
	{
	public:
		Register createRegister(int regId, int size, int64_t offset = 0x0) override;

		Register createFlagRegister(int flag) override;

		Register createInstructionPointerRegister() override;

		Register createStackPointerRegister() override;

	private:
		static Register CreateRegister(ZydisRegister reg, int size, int64_t offset = 0x0);

		static Register CreateFlagRegister(ZydisCPUFlag flag);
	};
};