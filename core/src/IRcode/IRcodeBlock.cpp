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

std::list<std::unique_ptr<Operation>>& Block::getOperations() {
    return m_operations;
}

Block* Block::getNearNextBlock() const {
    return m_function->toBlock(m_pcodeBlock->getNearNextBlock());
}

Block* Block::getFarNextBlock() const {
    return m_function->toBlock(m_pcodeBlock->getFarNextBlock());
}

std::list<Block*> Block::getReferencedBlocks() const {
    std::list<Block*> referencedBlocks;
    for (auto pcodeBlock : m_pcodeBlock->getReferencedBlocks()) {
        referencedBlocks.push_back(m_function->toBlock(pcodeBlock));
    }
    return referencedBlocks;
}

void Block::update() {

}