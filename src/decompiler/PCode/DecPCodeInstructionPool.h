#pragma once
#include "DecPCode.h"
#include <list>
#include <map>

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

		Instruction::OriginalInstruction* createOrigInstruction(int64_t offset, int length);

		Instruction* createInstruction(InstructionId id, Varnode* input0, Varnode* input1, Varnode* output, Instruction::OriginalInstruction* origInstr, int orderId = 0);

		void modifyInstruction(Instruction* instr, MODIFICATOR mod);

		// get pcode instruction at a complex offset (offset + order)
		Instruction* getInstructionAt(int64_t instrOffset);
	};
};