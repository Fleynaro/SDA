#include "Core/Pcode/PcodeBlock.h"

using namespace sda::pcode;

Block::Block(const std::map<InstructionOffset, Instruction>& instructions)
    : m_instructions(instructions)
{}

const std::map<InstructionOffset, Instruction>& Block::getInstructions() const {
    return m_instructions;
}