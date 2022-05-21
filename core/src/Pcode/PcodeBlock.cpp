#include "Core/Pcode/PcodeBlock.h"

using namespace sda::pcode;

Block::Block(const std::list<Instruction*>& instructions)
    : m_instructions(instructions)
{}

const std::list<Instruction*>& Block::getInstructions() const {
    return m_instructions;
}

Block* Block::getNearNextBlock() const {
    return m_nearNextBlock;
}

Block* Block::getFarNextBlock() const {
    return m_farNextBlock;
}