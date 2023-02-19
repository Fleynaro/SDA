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
    return m_graph->getFunctionGraphAt(getEntryBlock()->m_minOffset);
}

Block* Block::getEntryBlock() const {
    assert(isEntryBlockInited());
    return m_entryBlock;
}

bool Block::isEntryBlock() const {
    return getEntryBlock() == this;
}

bool Block::contains(InstructionOffset offset, bool halfInterval) const {
    if (offset == m_minOffset && offset == m_maxOffset) {
        // Block with no instructions
        return true;
    }
	return offset >= m_minOffset &&
        (halfInterval ? offset < m_maxOffset : offset <= m_maxOffset);
}

bool Block::canBeJoinedWith(Block* block) const {
    // blocks must be adjacent
    if (getMaxOffset() != block->getMinOffset()) {
        return false;
    }
    // check the last instruction
    auto lastInstr = getLastInstruction();
    if (lastInstr->isBranching() || lastInstr->isNotGoingNext()) {
        return false;
    }
    // no references to the 'block' from other blocks
    return block->getReferencedBlocks().empty() && block->getFunctionGraph()->getReferencesTo().empty();
}

void Block::passDescendants(std::function<void(Block* block, bool& goNextBlocks)> callback) {
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
            callback(block, goNextBlocks);
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

void Block::update() {
    if (!m_graph->m_updateBlocksEnabled) {
        return;
    }
    m_graph->m_updateBlocksEnabled = false;
    passDescendants([](Block* block, bool& goNextBlocks) {
        block->updateEntryBlocks(goNextBlocks);
    });
    m_graph->m_updateBlocksEnabled = true;
}

void Block::updateEntryBlocks(bool& goNextBlocks) {
    Block* newEntryBlock = nullptr;
    std::set<Block*> refEntryBlocks;
    for (auto refBlock : m_referencedBlocks) {
        if (refBlock->isEntryBlockInited()) {
            // the condition is needed to avoid excessive entry blocks (check tests NewEntryBlockWithLoopFirst, NewEntryBlockWithLoopSecond)
            if (refBlock->m_entryBlock != this) {
                refEntryBlocks.insert(refBlock->m_entryBlock);
            }
        }
    }
    /*
        The block is an entry block if:
        - it is the block that has no references (refEntryBlocks.size() == 0)
        - it is the block that has references from blocks in different functions (refEntryBlocks.size() > 1)
        - it is the block which another blocks transfer control to through a call (!curFunctionGraph->getReferencesTo().empty())
    */
    auto curFunctionGraph = m_graph->getFunctionGraphAt(m_minOffset);
    if (refEntryBlocks.size() != 1 || (curFunctionGraph && !curFunctionGraph->getReferencesTo().empty())) {
        newEntryBlock = this;
        if (!curFunctionGraph) {
            m_graph->createFunctionGraph(this);
        }
    }
    else {
        newEntryBlock = *refEntryBlocks.begin();
        if (curFunctionGraph) {
            auto newFunctionGraph = newEntryBlock->getFunctionGraph();
            if (curFunctionGraph != newFunctionGraph) {
                // check test NewCallSplitBlock
                curFunctionGraph->moveReferences(newFunctionGraph);
                m_graph->removeFunctionGraph(curFunctionGraph);
                curFunctionGraph = nullptr;
            }
        }
    }

    // check if need to update
    if (newEntryBlock == m_entryBlock)
        return;
    // update
    auto prevEntryBlock = m_entryBlock;
    m_entryBlock = newEntryBlock;
    if (prevEntryBlock) {
        auto prevFunctionGraph = prevEntryBlock->getFunctionGraph();
        auto newFunctionGraph = newEntryBlock->getFunctionGraph();
        if (prevFunctionGraph != newFunctionGraph) {
            // if the block becomes to belong to another func. graph then move all its "call" references to this new graph
            // check test NewCallSplitBlock
            prevFunctionGraph->moveReferences(newFunctionGraph, m_minOffset, m_maxOffset);
        }
    }
    goNextBlocks = true;
}

bool Block::isEntryBlockInited() const {
    return m_entryBlock && m_entryBlock->m_entryBlock;
}