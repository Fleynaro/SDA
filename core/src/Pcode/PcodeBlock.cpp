#include "SDA/Core/Pcode/PcodeBlock.h"
#include "SDA/Core/Pcode/PcodeGraph.h"
#include "SDA/Core/Pcode/PcodeEvents.h"
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

utils::BitSet Block::getDominantBlocksSet() const {
    return m_dominantBlocks;
}

std::list<Block*> Block::getDominantBlocks() const {
    auto funcGraph = getFunctionGraph();
    return funcGraph->toBlocks(m_dominantBlocks);
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
    assert(isEntryBlockInitialized());
    return m_entryBlock;
}

bool Block::isEntryBlock() const {
    return getEntryBlock() == this;
}

bool Block::hasSameEntryBlockAs(Block* block) const {
    return getEntryBlock() == block->getEntryBlock();
}

size_t Block::getIndex() const {
    return m_index;
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
    if (!m_graph->m_updateBlockEnabled)
        return;
    m_graph->m_updateBlockEnabled = false;
    // entry blocks & function graphs
    passDescendants([](Block* block, bool& goNextBlocks) {
        block->updateEntryBlocks(goNextBlocks);
    });
    // dominant blocks
    passDescendants([&](Block* block, bool& goNextBlocks) {
        if (block->m_dominantBlocks == utils::BitSet()) {
            return;
        }
        block->m_dominantBlocks.clear();
        goNextBlocks = true;
    });
    passDescendants([&](Block* block, bool& goNextBlocks) {
        block->updateDominantBlocks(goNextBlocks);
        m_graph->getEventPipe()->send(BlockUpdatedEvent {
            this,
            block == this
        });
    });
    m_graph->m_updateBlockEnabled = true;
}

void Block::updateDominantBlocks(bool& goNextBlocks) {
    assert(m_index != -1);
    utils::BitSet newDominantBlocks;
    newDominantBlocks.set(m_index, true);
    for (auto refBlock : m_referencedBlocks) {
        if (refBlock->hasSameEntryBlockAs(this)) { // check test TwoEntryPointsUnionSecond
            newDominantBlocks = newDominantBlocks | refBlock->m_dominantBlocks;
        }
    }
    // check if need to update
    if (newDominantBlocks == m_dominantBlocks)
        return;
    m_dominantBlocks = newDominantBlocks;
    goNextBlocks = true;
}

void Block::updateEntryBlocks(bool& goNextBlocks) {
    Block* newEntryBlock = nullptr;
    std::set<Block*> refEntryBlocks;
    for (auto refBlock : m_referencedBlocks) {
        if (refBlock->isEntryBlockInitialized()) {
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
                m_graph->removeFunctionGraph(curFunctionGraph); // >>> $1 <<<
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
    auto newFunctionGraph = newEntryBlock->getFunctionGraph();
    assert(newFunctionGraph);
    if (prevEntryBlock) {
        auto prevFunctionGraph = prevEntryBlock->getFunctionGraph();
        assert(prevFunctionGraph);
        if (prevFunctionGraph != newFunctionGraph) {
            // if the block becomes to belong to another func. graph then move all its "call" references to this new graph
            // check test NewCallSplitBlock
            // TODO: we could simplify this by implementing CALL references in block instead of function graph, along with JUMP references
            prevFunctionGraph->moveReferences(newFunctionGraph, m_minOffset, m_maxOffset);
            prevFunctionGraph->m_indexToBlock.erase(m_index);
            m_graph->getEventPipe()->send(BlockFunctionGraphChangedEvent {
                this, prevFunctionGraph, newFunctionGraph
            });
        } else {
            m_graph->getEventPipe()->send(BlockFunctionGraphChangedEvent {
                this, nullptr, newFunctionGraph
            });
            // if prevFunctionGraph == newFunctionGraph then the previous graph of the current block has been removed (see >>> $1 <<<)
        }
    } else {
        m_graph->getEventPipe()->send(BlockFunctionGraphChangedEvent {
            this, nullptr, newFunctionGraph
        });
    }
    // add the block to the new function graph
    m_index = FindNewIndex(newFunctionGraph->m_indexToBlock);
    newFunctionGraph->m_indexToBlock[m_index] = this;
    // go to next blocks
    goNextBlocks = true;
}

bool Block::isEntryBlockInitialized() const {
    return m_entryBlock && m_entryBlock->m_entryBlock;
}

size_t Block::FindNewIndex(const std::map<size_t, Block*>& indexToBlock) {
    auto blocksCount = indexToBlock.size();
    for (size_t i = blocksCount; i >= 0; --i) {
        if (indexToBlock.find(i) == indexToBlock.end()) {
            // found free index
            return i;
        }
    }
    throw std::runtime_error("Can't find free index");
}
