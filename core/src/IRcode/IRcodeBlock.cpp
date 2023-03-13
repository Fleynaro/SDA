#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeFunction.h"
#include "SDA/Core/Pcode/PcodeGraph.h"

using namespace sda;
using namespace sda::ircode;

Block::Block(pcode::Block* pcodeBlock, Function* function)
    : m_pcodeBlock(pcodeBlock), m_function(function)
{}

pcode::Block* Block::getPcodeBlock() const {
    return m_pcodeBlock;
}

std::string Block::getName() const {
    return m_pcodeBlock->getName();
}

size_t Block::getIndex() const {
    return m_pcodeBlock->getIndex();
}

std::list<std::unique_ptr<Operation>>& Block::getOperations() {
    return m_operations;
}

MemorySpace* Block::getMemorySpace() {
    return &m_memSpace;
}

Block* Block::getNearNextBlock() const {
    auto pcodeBlock = m_pcodeBlock->getNearNextBlock();
    if (!pcodeBlock) {
        return nullptr;
    }
    return m_function->toBlock(pcodeBlock);
}

Block* Block::getFarNextBlock() const {
    auto pcodeBlock = m_pcodeBlock->getFarNextBlock();
    if (!pcodeBlock) {
        return nullptr;
    }
    return m_function->toBlock(pcodeBlock);
}

std::list<Block*> Block::getReferencedBlocks() const {
    std::list<Block*> referencedBlocks;
    for (auto pcodeBlock : m_pcodeBlock->getReferencedBlocks()) {
        referencedBlocks.push_back(m_function->toBlock(pcodeBlock));
    }
    return referencedBlocks;
}

void Block::passDescendants(std::function<void(Block* block, bool& goNextBlocks)> callback) {
    m_pcodeBlock->passDescendants([this, &callback](pcode::Block* pcodeBlock, bool& goNextBlocks) {
        callback(m_function->toBlock(pcodeBlock), goNextBlocks);
    });
}

void Block::update() {
    passDescendants([&](Block* block, bool& goNextBlocks) {
        block->decompile(goNextBlocks);
    });
}

void Block::decompile(bool& goNextBlocks) {
    auto& instructions = m_pcodeBlock->getInstructions();
    for (auto& [offset, instruction] : instructions) {
        
    }
    goNextBlocks = false;
}
