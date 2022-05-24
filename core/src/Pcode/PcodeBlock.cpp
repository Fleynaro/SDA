#include "Core/Pcode/PcodeBlock.h"

using namespace sda::pcode;

Block::Block(InstructionOffset minOffset)
    : m_minOffset(minOffset), m_maxOffset(minOffset)
{}

std::list<const Instruction*>& Block::getInstructions() {
    return m_instructions;
}

void Block::setNearNextBlock(Block* nearNextBlock) {
    nearNextBlock->m_referencedBlocks.remove(m_nearNextBlock);
    nearNextBlock->m_referencedBlocks.push_back(nearNextBlock);
    m_nearNextBlock = nearNextBlock;
}

Block* Block::getNearNextBlock() const {
    return m_nearNextBlock;
}

void Block::setFarNextBlock(Block* farNextBlock) {
    farNextBlock->m_referencedBlocks.remove(m_farNextBlock);
    farNextBlock->m_referencedBlocks.push_back(farNextBlock);
    m_farNextBlock = farNextBlock;
}

Block* Block::getFarNextBlock() const {
    return m_farNextBlock;
}

const std::list<Block*>& Block::getReferencedBlocks() const {
    return m_referencedBlocks;
}

InstructionOffset Block::getMinOffset() const {
    return m_minOffset;
}

void Block::setMaxOffset(InstructionOffset maxOffset) {
    m_maxOffset = maxOffset;
}

InstructionOffset Block::getMaxOffset() const {
    return m_maxOffset;
}

bool Block::contains(InstructionOffset offset, bool halfInterval) const {
	return offset >= m_minOffset &&
        (halfInterval ? offset < m_maxOffset : offset <= m_maxOffset);
}