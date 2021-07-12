#include "DecPCodeInstructionPool.h"

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::PCode;

RegisterVarnode* InstructionPool::createRegisterVarnode(Register reg) {
	m_registerVarnodes.push_back(RegisterVarnode(reg));
	return &*m_registerVarnodes.rbegin();
}

ConstantVarnode* InstructionPool::createConstantVarnode(uint64_t value, int size) {
	m_constantVarnodes.push_back(ConstantVarnode(value, size));
	return &*m_constantVarnodes.rbegin();
}

Instruction::OriginalInstruction* InstructionPool::getOrigInstructionAt(uint64_t offset) {
	auto it = m_origInstructions.find(offset);
	if (it == m_origInstructions.end())
		return nullptr;
	return &it->second;
}

Instruction* InstructionPool::getPCodeInstructionAt(uint64_t instrOffset) {
	const auto byteOffset = instrOffset >> 8;
	const auto instrOrder = static_cast<int>(instrOffset & 0xFF);
	auto origInstr = getOrigInstructionAt(byteOffset);
	if(!origInstr)
		return nullptr;
	auto it = origInstr->m_pcodeInstructions.find(instrOrder);
	if (it == origInstr->m_pcodeInstructions.end())
		return nullptr;
	return &it->second;
}

void InstructionPool::modifyInstruction(Instruction* instr, MODIFICATOR mod) {
	switch (mod)
	{
	case MODIFICATOR_JMP_CALL:
		// replace JMP with CALL and add RET
		instr->m_id = PCode::InstructionId::CALL;
		createInstruction(PCode::InstructionId::RETURN, nullptr, nullptr, nullptr, instr->m_origInstruction, 1);
		break;
	}
	m_modifiedInstructions[instr->getOffset()] = mod;
}

SymbolVarnode* InstructionPool::createSymbolVarnode(int size) {
	m_symbolVarnodes.push_back(SymbolVarnode(size));
	return &*m_symbolVarnodes.rbegin();
}

Instruction::OriginalInstruction* CE::Decompiler::PCode::InstructionPool::createOrigInstruction(uint64_t offset, int length) {
	m_origInstructions[offset] = Instruction::OriginalInstruction(offset, length);
	return &m_origInstructions[offset];
}

Instruction* CE::Decompiler::PCode::InstructionPool::createInstruction(InstructionId id, Varnode* input0, Varnode* input1, Varnode* output, Instruction::OriginalInstruction* origInstr, int orderId) {
	auto instr = Instruction(id, input0, input1, output, origInstr, orderId);
	origInstr->m_pcodeInstructions[orderId] = instr;
	// check if can modificate the instruction
	const auto it = m_modifiedInstructions.find(instr.getOffset());
	if (it != m_modifiedInstructions.end()) {
		modifyInstruction(&instr, it->second);
	}
	return &origInstr->m_pcodeInstructions[orderId];
}
