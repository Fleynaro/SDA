#include "DecPCode.h"

using namespace CE::Decompiler;
using namespace PCode;

Register::Register(RegisterId genericId, int index, BitMask64 valueRangeMask, Type type): m_genericId(genericId),
	m_index(index), m_valueRangeMask(valueRangeMask), m_type(type) {
	m_debugName = InstructionViewGenerator::GenerateRegisterName(*this);
}

Register::Type Register::getType() const {
	return m_type;
}

int Register::getId() const {
	return (m_genericId << 8) | m_index;
}

RegisterId Register::getGenericId() const {
	return m_genericId;
}

int Register::getIndex() const {
	return m_index;
}

bool Register::isValid() const {
	return m_genericId != 0;
}

bool Register::isPointer() const {
	return m_type == Type::StackPointer || m_type == Type::InstructionPointer;
}

bool Register::isVector() const {
	return m_type == Type::Vector;
}

// get size (in bytes) of values range

int Register::getSize() const {
	return m_valueRangeMask.getSize();
}

int Register::getOffset() const {
	return m_valueRangeMask.getOffset() + m_index * 64;
}

// check if memory area of two registers intersected

bool Register::intersect(const Register& reg) const {
	//if the masks intersected
	return getId() == reg.getId() && !(m_valueRangeMask & reg.m_valueRangeMask).isZero();
}

// that is the feature of x86: setting value to EAX cleans fully RAX
BitMask64 PCode::GetValueRangeMaskWithException(const Register& reg) {
	if (reg.getType() == Register::Type::Helper && reg.m_valueRangeMask == BitMask64(4))
		return BitMask64(8);
	return reg.m_valueRangeMask;
}

// get long offset which consist of original offset and pCode instruction order number: origOffset{24} | order{8}

CE::ComplexOffset Instruction::getOffset() const
{
	return ComplexOffset(m_origInstruction->m_offset, m_orderId);
}

// get long offset of the next instruction following this

CE::ComplexOffset Instruction::getFirstInstrOffsetInNextOrigInstr() const
{
	return ComplexOffset(m_origInstruction->m_offset + m_origInstruction->m_length, 0);
}

// BRANCH, CBRANCH, BRANCHIND

bool Instruction::IsBranching(InstructionId id) {
	return id >= InstructionId::BRANCH && id <= InstructionId::BRANCHIND;
}

// check if the instruction is some kind of jump (BRANCH/CALL/RETURN)

bool Instruction::IsAnyJmup(InstructionId id) {
	return id >= InstructionId::BRANCH && id <= InstructionId::RETURN;
}

int SymbolVarnode::getSize() {
	return m_size;
}

int ConstantVarnode::getSize() {
	return m_size;
}

int RegisterVarnode::getSize() {
	return m_register.getSize();
}

BitMask64 RegisterVarnode::getMask() {
	return m_register.m_valueRangeMask;
}
