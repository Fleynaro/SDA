#include "SDA/Core/Pcode/PcodeInstruction.h"

using namespace sda::pcode;

InstructionOffset::InstructionOffset(Offset offset)
    : fullOffset(offset)
{}

InstructionOffset::InstructionOffset(Offset byteOffset, size_t index)
    : byteOffset(byteOffset), index(index)
{}

InstructionOffset::operator size_t() const {
    return fullOffset;
}

size_t InstructionOffset::GetMaxIndex() {
    return 255;
}

Instruction::Instruction(
    InstructionId id,
    std::shared_ptr<Varnode> input0,
    std::shared_ptr<Varnode> input1,
    std::shared_ptr<Varnode> output,
    InstructionOffset offset)
    : m_id(id), m_input0(input0), m_input1(input1), m_output(output), m_offset(offset)
{}

InstructionId Instruction::getId() const {
    return m_id;
}

std::shared_ptr<Varnode> Instruction::getInput0() const {
    return m_input0;
}

std::shared_ptr<Varnode> Instruction::getInput1() const {
    return m_input1;
}

std::shared_ptr<Varnode> Instruction::getOutput() const {
    return m_output;
}

InstructionOffset Instruction::getOffset() const {
    return m_offset;
}

bool Instruction::isAnyJump() const {
    return m_id >= InstructionId::BRANCH && m_id <= InstructionId::RETURN;
}

bool Instruction::isBranching() const {
    return m_id >= InstructionId::BRANCH && m_id <= InstructionId::BRANCHIND;
}

bool Instruction::isComutative() const {
    return
        // integers
        m_id == InstructionId::INT_ADD ||
        m_id == InstructionId::INT_MULT ||
        m_id == InstructionId::INT_XOR ||
        m_id == InstructionId::INT_AND ||
        m_id == InstructionId::INT_OR ||
        m_id == InstructionId::INT_EQUAL ||
        m_id == InstructionId::INT_NOTEQUAL ||
        // floats
        m_id == InstructionId::FLOAT_ADD ||
        m_id == InstructionId::FLOAT_MULT ||
        m_id == InstructionId::FLOAT_EQUAL ||
        m_id == InstructionId::FLOAT_NOTEQUAL;
}