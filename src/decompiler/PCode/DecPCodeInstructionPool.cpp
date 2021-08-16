#include "DecPCodeInstructionPool.h"

using namespace CE;
using namespace Decompiler;
using namespace PCode;

RegisterVarnode* InstructionPool::createRegisterVarnode(Register reg) {
	m_registerVarnodes.emplace_back(reg);
	return &*m_registerVarnodes.rbegin();
}

ConstantVarnode* InstructionPool::createConstantVarnode(uint64_t value, int size, bool isAddr) {
	m_constantVarnodes.emplace_back(value, size, isAddr);
	return &*m_constantVarnodes.rbegin();
}

Instruction::OriginalInstruction* InstructionPool::getOrigInstructionAt(Offset offset) {
	auto it = m_origInstructions.find(offset);
	if (it == m_origInstructions.end())
		return nullptr;
	return &it->second;
}

Instruction* InstructionPool::getPCodeInstructionAt(ComplexOffset instrOffset) {
	auto origInstr = getOrigInstructionAt(instrOffset.getByteOffset());
	if(!origInstr)
		return nullptr;
	auto it = origInstr->m_pcodeInstructions.find(instrOffset.getOrderId());
	if (it == origInstr->m_pcodeInstructions.end())
		return nullptr;
	return &it->second;
}

void InstructionPool::modifyInstruction(Instruction* instr, MODIFICATOR mod) {
	switch (mod)
	{
	case MODIFICATOR_JMP_CALL:
		// replace JMP with CALL and add RET
		instr->m_id = InstructionId::CALL;
		createInstruction(InstructionId::RETURN, nullptr, nullptr, nullptr, instr->m_origInstruction, 1);
		break;
	}
	m_modifiedInstructions[instr->getOffset()] = mod;
}

SymbolVarnode* InstructionPool::createSymbolVarnode(int size) {
	m_symbolVarnodes.emplace_back(size);
	return &*m_symbolVarnodes.rbegin();
}

Instruction::OriginalInstruction* InstructionPool::createOrigInstruction(Offset offset, int length) {
	m_origInstructions[offset] = Instruction::OriginalInstruction(offset, length);
	return &m_origInstructions[offset];
}

Instruction* InstructionPool::createInstruction(InstructionId id, Varnode* input0, Varnode* input1, Varnode* output, Instruction::OriginalInstruction* origInstr, int orderId) {
	auto instr = Instruction(id, input0, input1, output, origInstr, orderId);
	origInstr->m_pcodeInstructions[orderId] = instr;
	// check if can modificate the instruction
	const auto it = m_modifiedInstructions.find(instr.getOffset());
	if (it != m_modifiedInstructions.end()) {
		modifyInstruction(&instr, it->second);
	}
	return &origInstr->m_pcodeInstructions[orderId];
}
