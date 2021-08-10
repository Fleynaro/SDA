#pragma once
#include "../DecRegisterFactory.h"
#include "../DecPCodeInstructionPool.h"
#include <vector>
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

		void decode(Offset offset, const std::vector<uint8_t>& data);

		void clear();

		std::list<Instruction*>& getDecodedPCodeInstructions();

		void deleteDecodedPCodeInstructions();

		Instruction::OriginalInstruction* getOrigInstruction() const;

		WarningContainer* getWarningContainer() override;
	protected:
		WarningContainer* m_warningContainer;
		std::list<Instruction*> m_result;
		Instruction::OriginalInstruction* m_curOrigInstr = nullptr;
		Offset m_curOffset = 0;
		int m_curOrderId = 0;

		virtual void tryDecode(const std::vector<uint8_t>& data) = 0;
	};
};