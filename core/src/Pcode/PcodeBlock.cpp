#include "SDA/Core/Pcode/PcodeBlock.h"
#include "SDA/Core/Pcode/PcodeGraph.h"
#include "SDA/Core/Utils/IOManip.h"

using namespace sda::pcode;

Block::Block(Graph* graph, InstructionOffset minOffset)
    : m_graph(graph), m_minOffset(minOffset), m_maxOffset(minOffset)
{}

std::string Block::getName() const {
    auto name = std::stringstream() << "B" << utils::to_hex() << (m_minOffset.byteOffset & 0xFFFF);
    return name.str();
}

std::map<InstructionOffset, const Instruction*>& Block::getInstructions() {
    return m_instructions;
}

void Block::setNearNextBlock(Block* nearNextBlock) {
    if (nearNextBlock) {
        nearNextBlock->m_referencedBlocks.remove(m_nearNextBlock);
        nearNextBlock->m_referencedBlocks.push_back(nearNextBlock);
    }
    m_nearNextBlock = nearNextBlock;
}

Block* Block::getNearNextBlock() const {
    return m_nearNextBlock;
}

void Block::setFarNextBlock(Block* farNextBlock) {
    if (farNextBlock) {
        farNextBlock->m_referencedBlocks.remove(m_farNextBlock);
        farNextBlock->m_referencedBlocks.push_back(farNextBlock);
    }
    m_farNextBlock = farNextBlock;
}

Block* Block::getFarNextBlock() const {
    return m_farNextBlock;
}

std::list<Block*> Block::getNextBlocks() const {
    std::list<Block*> nextBlocks;
    if (m_nearNextBlock) {
        nextBlocks.push_back(m_nearNextBlock);
    }
    if (m_farNextBlock) {
        nextBlocks.push_back(m_farNextBlock);
    }
    return nextBlocks;
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

FunctionGraph* Block::getFunctionGraph() const {
    return m_functionGraph;
}

size_t Block::getLevel() const {
    return m_level;
}

bool Block::contains(InstructionOffset offset, bool halfInterval) const {
	return offset >= m_minOffset &&
        (halfInterval ? offset < m_maxOffset : offset <= m_maxOffset);
}

bool Block::isInited() const {
    return m_level != 0;
}

bool Block::hasLoop() const {
    if (!m_farNextBlock)
        return false;
    if (!isInited())
        throw std::runtime_error("Block::hasLoop: block is not inited");
    if (!m_farNextBlock->isInited())
        throw std::runtime_error("Block::hasLoop: far next block is not inited");
    return m_farNextBlock->m_level <= m_level;
}

void Block::update() {
    size_t newLevel = 1;
    std::list<Block*> refBlocks;
    for (auto refBlock : m_referencedBlocks) {
        if (refBlock->isInited() && !refBlock->hasLoop()) {
            refBlocks.push_back(refBlock);
        }
    }
    for (auto it = refBlocks.begin(); it != refBlocks.end(); ++it) {
        auto refBlock = *it;
        // calculate level
        if (!refBlock->hasLoop()) {
            newLevel = std::max(newLevel, refBlock->m_level + 1);
        }
        // check if are ref. blocks of the same function graph
        auto nextIt = std::next(it);
        if (nextIt != refBlocks.end()) {
            auto nextRefBlock = *nextIt;
            if (refBlock->m_functionGraph != nextRefBlock->m_functionGraph) {
                
            }
        }
    }

    if (newLevel == m_level)
        return;
    m_level = newLevel;

    if (m_nearNextBlock)
        m_nearNextBlock->update();
    if (m_farNextBlock && !hasLoop())
        m_farNextBlock->update();
}