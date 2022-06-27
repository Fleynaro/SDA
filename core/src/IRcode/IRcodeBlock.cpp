#include "Core/IRcode/IRcodeBlock.h"

using namespace sda::ircode;

Block::Block(pcode::Block* pcodeBlock)
    : m_pcodeBlock(pcodeBlock)
{}

std::list<Operation>& Block::getOperations() {
    return m_operations;
}

Block* Block::getNearNextBlock() const {
    return m_nearNextBlock;
}

Block* Block::getFarNextBlock() const {
    return m_farNextBlock;
}

void Block::setNextBlocks(Block* nearNextBlock, Block* farNextBlock) {
    // near next block
    m_nearNextBlock = nearNextBlock;
    m_nearNextBlock->m_previousBlocks.remove(this);
    m_nearNextBlock->m_previousBlocks.push_back(this);

    // far next block
    m_farNextBlock = farNextBlock;
    m_farNextBlock->m_previousBlocks.remove(this);
    m_farNextBlock->m_previousBlocks.push_back(this);
}

const std::list<Block*>& Block::getPreviousBlocks() const {
    return m_previousBlocks;
}