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

const Instruction* Block::getLastInstruction() const {
    return m_instructions.empty() ? nullptr : m_instructions.rbegin()->second;
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
    assert(m_inited);
    return m_graph->getFunctionGraphAt(m_entryBlock->m_minOffset);
}

Block* Block::getEntryBlock() const {
    assert(m_inited);
    return m_entryBlock;
}

size_t Block::getLevel() const {
    assert(m_inited);
    return m_level;
}

bool Block::contains(InstructionOffset offset, bool halfInterval) const {
    if (offset == m_minOffset && offset == m_maxOffset) {
        // Block with no instructions
        return true;
    }
	return offset >= m_minOffset &&
        (halfInterval ? offset < m_maxOffset : offset <= m_maxOffset);
}

bool Block::hasLoopWith(Block* block) const {
    assert(m_inited);
    assert(block->m_inited);
    return m_farNextBlock && block == m_farNextBlock && block->m_level <= m_level;
}

bool Block::canReach(Block* blockToReach) const {
    std::set<const Block*> visitedBlocks;
    std::list<const Block*> toVisit;
    toVisit.push_back(this);
    while (!toVisit.empty()) {
        auto block = toVisit.front();
        toVisit.pop_front();
        if (visitedBlocks.find(block) != visitedBlocks.end()) {
            continue;
        }
        visitedBlocks.insert(block);
        for (auto nextBlock : block->getNextBlocks()) {
            if (nextBlock == blockToReach) {
                return true;
            }
            // hasLoopWith is here for optimization only
            if (!block->hasLoopWith(nextBlock)) {
                toVisit.push_back(nextBlock);
            }
        }
    }
    return false;
}

void Block::update() {
    if (!m_graph->m_updateBlocksEnabled) {
        return;
    }
    m_graph->m_updateBlocksEnabled = false;
    update(&Block::updateLevels);
    update(&Block::updateEntryBlocks);
    m_graph->m_updateBlocksEnabled = true;
}

void Block::update(void (Block::*updateMethod)(bool& goNextBlocks)) {
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
            bool goNextBlocks = false;
            (block->*updateMethod)(goNextBlocks);
            if (goNextBlocks) {
                for (auto nextBlock : block->getNextBlocks()) {
                    blocksToVisit.push_back(nextBlock);
                }
            }
        }
        if (!blockKnocks.empty()) {
            auto block = blockKnocks.begin()->first;
            blocksToVisit.push_back(block);
        }
    } while (!blocksToVisit.empty());
}

void Block::updateLevels(bool& goNextBlocks) {
    size_t newLevel = 1;
    for (auto refBlock : m_referencedBlocks) {
        if (refBlock->m_inited) {
            // canReach is used to check if there is a loop
            if (!m_inited || !canReach(refBlock)) {
                newLevel = std::max(newLevel, refBlock->m_level + 1);
            }
        }
    }
    // check if need to update
    if (m_inited && newLevel == m_level)
        return;
    // update
    m_level = newLevel;
    m_inited = true;
    goNextBlocks = true;
}

void Block::updateEntryBlocks(bool& goNextBlocks) {
    Block* newEntryBlock = nullptr;
    std::set<Block*> refEntryBlocks;
    for (auto refBlock : m_referencedBlocks) {
        if (refBlock->m_inited) {
            refEntryBlocks.insert(refBlock->m_entryBlock);
        }
    }
    auto curFunctionGraph = m_graph->getFunctionGraphAt(m_minOffset);
    if (refEntryBlocks.size() == 1) {
        newEntryBlock = *refEntryBlocks.begin();
        if (curFunctionGraph && curFunctionGraph != newEntryBlock->getFunctionGraph()) {
            m_graph->removeFunctionGraph(curFunctionGraph);
        }
    }
    else {
        newEntryBlock = this;
        if (!curFunctionGraph) {
            m_graph->createFunctionGraph(this);
        }
    }

    // check if need to update
    if (m_inited && newEntryBlock == m_entryBlock)
        return;
    // update
    m_entryBlock = newEntryBlock;
    m_inited = true;
    goNextBlocks = true;
}