#include "DecPCode.h"
//for debug x86
#include <Zycore/Format.h>
#include <Zycore/LibC.h>

using namespace CE::Decompiler;
using namespace PCode;

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

std::string Register::printDebug() const
{
	const auto regId = static_cast<ZydisRegister>(m_genericId);

	const auto size = getSize();
	std::string maskStr = std::to_string(size);
	if (isVector()) {
		if (size == 4 || size == 8) {
			maskStr = std::string(size == 4 ? "D" : "Q") + static_cast<char>('a' + (char)(getOffset() / (size * 8)));
		}
	}

	if (regId != ZYDIS_REGISTER_RFLAGS)
		return std::string(ZydisRegisterGetString(regId)) + ":" + maskStr;

	std::string flagName = "flag";
	const auto flag = static_cast<ZydisCPUFlag>(m_valueRangeMask.getOffset());
	if (flag == ZYDIS_CPUFLAG_CF)
		flagName = "CF";
	else if (flag == ZYDIS_CPUFLAG_OF)
		flagName = "OF";
	else if (flag == ZYDIS_CPUFLAG_SF)
		flagName = "SF";
	else if (flag == ZYDIS_CPUFLAG_ZF)
		flagName = "ZF";
	else if (flag == ZYDIS_CPUFLAG_AF)
		flagName = "AF";
	else if (flag == ZYDIS_CPUFLAG_PF)
		flagName = "PF";
	return flagName + ":1";
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

std::string Instruction::printDebug() const
{
	std::string result;
	if (m_output)
		result += m_output->printDebug() + " = ";
	result += magic_enum::enum_name(m_id);
	if (m_input0)
		result += " " + m_input0->printDebug();
	if (m_input1)
		result += ", " + m_input1->printDebug();
	return result;
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

std::string SymbolVarnode::printDebug() {
	return "$U" + std::to_string((uint64_t)this % 10000) + ":" + std::to_string(getSize());
}

int ConstantVarnode::getSize() {
	return m_size;
}

std::string ConstantVarnode::printDebug() {
	return std::to_string((int64_t&)m_value) + ":" + std::to_string(getSize());
}

int RegisterVarnode::getSize() {
	return m_register.getSize();
}

BitMask64 RegisterVarnode::getMask() {
	return m_register.m_valueRangeMask;
}

std::string RegisterVarnode::printDebug() {
	return m_register.printDebug();
}
