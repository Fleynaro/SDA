#pragma once
#include "../DecRegisterFactory.h"
#include "../DecPCodeInstructionPool.h"
#include "../../DecWarningContainer.h"

namespace CE::Decompiler::PCode
{
	class AbstractDecoder : public IWarningGenerator
	{
	public:
		InstructionPool* m_instrPool;

		AbstractDecoder(InstructionPool* instrPool, WarningContainer* warningContainer)
			: m_instrPool(instrPool), m_warningContainer(warningContainer)
		{}

		void decode(void* addr, int offset, int maxSize = 0x0) {
			m_addr = addr;
			m_curOrigInstr = nullptr;
			m_curOrderId = 0x0;
			m_maxSize = maxSize;
			clear();
			tryDecode(addr, offset);
		}

		void clear() {
			m_result.clear();
		}

		std::list<Instruction*>& getDecodedPCodeInstructions() {
			return m_result;
		}

		void deleteDecodedPCodeInstructions() {
			for (auto instr : getDecodedPCodeInstructions()) {
				delete instr;
			}
		}

		Instruction::OriginalInstruction* getOrigInstruction() {
			return m_curOrigInstr;
		}

		WarningContainer* getWarningContainer() override {
			return m_warningContainer;
		}
	protected:
		WarningContainer* m_warningContainer;
		std::list<Instruction*> m_result;
		void* m_addr = nullptr;
		Instruction::OriginalInstruction* m_curOrigInstr;
		int m_curOrderId = 0;
		int m_maxSize = 0x0;

		virtual void tryDecode(void* addr, int offset) = 0;
	};
};