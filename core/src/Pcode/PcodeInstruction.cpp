#include "Core/Pcode/PcodeInstruction.h"

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

Instruction::Instruction(
    InstructionId id,
    std::shared_ptr<Varnode> input0,
    std::shared_ptr<Varnode> input1,
    std::shared_ptr<Varnode> output)
    : m_id(id), m_input0(input0), m_input1(input1), m_output(output)
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