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

Graph* Block::getGraph() {
    return m_graph;
}

std::map<InstructionOffset, const Instruction*>& Block::getInstructions() {
    return m_instructions;
}

void Block::setNearNextBlock(Block* nearNextBlock) {
    auto prevNearNextBlock = m_nearNextBlock;
    if (prevNearNextBlock) {
        prevNearNextBlock->m_referencedBlocks.remove(this);
    }
    if (nearNextBlock) {
        nearNextBlock->m_referencedBlocks.push_back(this);
    }
    m_nearNextBlock = nearNextBlock;
    if (prevNearNextBlock) {
        prevNearNextBlock->update();
    }
    if (nearNextBlock) {
        nearNextBlock->update();
    }
}

Block* Block::getNearNextBlock() const {
    return m_nearNextBlock;
}

void Block::setFarNextBlock(Block* farNextBlock) {
    auto prevFarNextBlock = m_farNextBlock;
    if (prevFarNextBlock) {
        prevFarNextBlock->m_referencedBlocks.remove(this);
    }
    if (farNextBlock) {
        farNextBlock->m_referencedBlocks.push_back(this);
    }
    m_farNextBlock = farNextBlock;
    if (prevFarNextBlock) {
        prevFarNextBlock->update();
    }
    if (farNextBlock) {
        farNextBlock->update();
    }
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
    // check if blocks are inited
    assert(isInited() && m_farNextBlock->isInited());
    return m_farNextBlock->m_level <= m_level;
}

void Block::update() {
    std::map<Block*, size_t> blockKnocks;
    std::list<Block*> blocksToVisit;
    blocksToVisit.push_back(this);
    do {
        while (!blocksToVisit.empty()) {
            auto block = blocksToVisit.front();
            blocksToVisit.pop_front();
            auto it = blockKnocks.find(block);
            if (it == blockKnocks.end()) {
                it = blockKnocks.insert({ block, 0 }).first;
            }
            auto knocks = ++it->second;
            if (knocks < block->getReferencedBlocks().size()) {
                continue;
            }
            blockKnocks.erase(it);
            block->update(blocksToVisit);
        }
        if (!blockKnocks.empty()) {
            auto block = blockKnocks.begin()->first;
            blocksToVisit.push_back(block);
        }
    } while (!blocksToVisit.empty());
}

void Block::update(std::list<Block*>& nextBlocks) {
    // get inited referenced blocks
    std::list<Block*> refBlocks;
    for (auto refBlock : m_referencedBlocks) {
        if (refBlock->isInited() && !refBlock->hasLoop()) {
            refBlocks.push_back(refBlock);
        }
    }

    size_t newLevel = 1;
    bool needNewFunctionGraph = false;
    Block* prevRefBlock = nullptr;
    for (auto refBlock : refBlocks) {
        // calculate level
        newLevel = std::max(newLevel, refBlock->m_level + 1);
        // check if are ref. blocks of the same function graph
        if (!needNewFunctionGraph) {
            if (prevRefBlock && refBlock->m_functionGraph != prevRefBlock->m_functionGraph) {
                needNewFunctionGraph = true;
            }
        }
        prevRefBlock = refBlock;
    }

    // create new function graph
    FunctionGraph* newFunctionGraph = nullptr;
    if (needNewFunctionGraph) {
        newFunctionGraph = m_graph->createFunctionGraph(this, false);
        for (auto refBlock : m_referencedBlocks) {
            if (!refBlock->isInited())
                continue;
            refBlock->m_jumpToFunction = true;
        }
    } else {
        auto curFunctionGraph = m_graph->getFunctionGraphAt(m_minOffset);
        if (refBlocks.empty()) {
            newFunctionGraph = curFunctionGraph;
        } else {
            if (curFunctionGraph) {
                m_graph->removeFunctionGraph(curFunctionGraph, false);
                for (auto refBlock : m_referencedBlocks) {
                    if (!refBlock->isInited())
                        continue;
                    refBlock->m_jumpToFunction = false;
                }
            }
            newFunctionGraph = refBlocks.front()->m_functionGraph;
        }
    }

    // check if need to update
    if (newLevel == m_level && newFunctionGraph == m_functionGraph)
        return;
    // update
    m_level = newLevel;
    m_functionGraph = newFunctionGraph;

    // go to next blocks
    if (m_nearNextBlock)
        nextBlocks.push_back(m_nearNextBlock);
    if (m_farNextBlock && !hasLoop())
        nextBlocks.push_back(m_farNextBlock);
}