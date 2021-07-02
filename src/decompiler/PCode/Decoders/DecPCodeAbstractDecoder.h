#pragma once
#include "../DecRegisterFactory.h"
#include "../DecPCodeInstructionPool.h"
#include <decompiler/DecWarningContainer.h>

namespace CE::Decompiler::PCode
{
	class AbstractDecoder : public IWarningGenerator
	{
	public:
		InstructionPool* m_instrPool;

		AbstractDecoder(InstructionPool* instrPool, WarningContainer* warningContainer)
			: m_instrPool(instrPool), m_warningContainer(warningContainer)
		{}

		void decode(void* addr, int offset, int maxSize = 0x0);

		void clear();

		std::list<Instruction*>& getDecodedPCodeInstructions();

		void deleteDecodedPCodeInstructions();

		Instruction::OriginalInstruction* getOrigInstruction();

		WarningContainer* getWarningContainer() override;
	protected:
		WarningContainer* m_warningContainer;
		std::list<Instruction*> m_result;
		void* m_addr = nullptr;
		Instruction::OriginalInstruction* m_curOrigInstr = nullptr;
		int m_curOrderId = 0;
		int m_maxSize = 0x0;

		virtual void tryDecode(void* addr, int offset) = 0;
	};
};