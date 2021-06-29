#pragma once
#include "DecPCode.h"

namespace CE::Decompiler::PCode
{
	class InstructionPool
	{
		std::list<RegisterVarnode> m_registerVarnodes;
		std::list<ConstantVarnode> m_constantVarnodes;
		std::list<SymbolVarnode> m_symbolVarnodes;
		std::map<int64_t, Instruction::OriginalInstruction> m_origInstructions;
	public:
		// exceptions
		class InstructionNotFoundException : public std::exception {};

		// some orig. instruction can be changed during image analysis (JMP -> CALL/RET)
		enum MODIFICATOR {
			MODIFICATOR_JMP_CALL
		};
		std::map<int64_t, MODIFICATOR> m_modifiedInstructions;

		InstructionPool()
		{}

		RegisterVarnode* createRegisterVarnode(Register reg);

		ConstantVarnode* createConstantVarnode(uint64_t value, int size);

		SymbolVarnode* createSymbolVarnode(int size);

		Instruction::OriginalInstruction* createOrigInstruction(int64_t offset, int length) {
			m_origInstructions[offset] = Instruction::OriginalInstruction(offset, length);
			return &m_origInstructions[offset];
		}

		Instruction* createInstruction(InstructionId id, Varnode* input0, Varnode* input1, Varnode* output, Instruction::OriginalInstruction* origInstr, int orderId = 0) {
			auto instr = Instruction(id, input0, input1, output, origInstr, orderId);
			origInstr->m_pcodeInstructions[orderId] = instr;
			// check if can modificate the instruction
			auto it = m_modifiedInstructions.find(instr.getOffset());
			if (it != m_modifiedInstructions.end()) {
				modifyInstruction(&instr, it->second);
			}
			return &origInstr->m_pcodeInstructions[orderId];
		}

		void modifyInstruction(Instruction* instr, MODIFICATOR mod);

		// get pcode instruction at a complex offset (offset + order)
		Instruction* getInstructionAt(int64_t instrOffset);
	};
};