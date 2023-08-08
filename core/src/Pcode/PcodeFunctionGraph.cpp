#include "SDA/Core/Pcode/PcodeFunctionGraph.h"
#include "SDA/Core/Pcode/PcodeGraph.h"

using namespace sda::pcode;

FunctionGraph::FunctionGraph(Block* entryBlock)
    : m_entryBlock(entryBlock)
{}

std::string FunctionGraph::getName() const {
    return getEntryBlock()->getName();
}

Block* FunctionGraph::getEntryBlock() const {
    return m_entryBlock;
}

sda::Offset FunctionGraph::getEntryOffset() const {
    return m_entryBlock->getMinOffset();
}

void CalculateLevels(Block* entryBlock, Block* block, std::list<Block*>& path, std::map<Block*, size_t>& blockToLevel) {
    if (block->getEntryBlock() != entryBlock) return;
    for (auto it = path.rbegin(); it != path.rend(); ++it) {
        if (*it == block) {
            return;
        }
    }
    path.push_back(block);
    for (auto nextBlock : block->getNextBlocks()) {
        CalculateLevels(entryBlock, nextBlock, path, blockToLevel);
    }
    auto it = blockToLevel.find(block);
    if (it != blockToLevel.end()) {
        it->second = std::max(it->second, path.size());
    } else {
        blockToLevel[block] = path.size();
    }
    path.pop_back();
}

std::list<FunctionGraph::BlockInfo> FunctionGraph::getBlocks(bool sort) const {
    std::map<Block*, size_t> blockToLevel;
    std::list<Block*> path;
    CalculateLevels(m_entryBlock, m_entryBlock, path, blockToLevel);
    std::list<BlockInfo> blocks;
    for (auto& it : blockToLevel) {
        blocks.push_back({ it.first, it.second });
    }
    // sort blocks by level and offset
    if (sort) {
        blocks.sort([](const FunctionGraph::BlockInfo& a, const FunctionGraph::BlockInfo& b) {
            if (a.level != b.level)
                return a.level < b.level;
            return a.block->getMinOffset() < b.block->getMinOffset();
        });
    }
    return blocks;
}

Graph* FunctionGraph::getGraph() {
    return m_entryBlock->getGraph();
}

const std::list<FunctionGraph*>& FunctionGraph::getReferencesTo() const {
    return m_referencedGraphsTo;
}

const std::map<InstructionOffset, FunctionGraph*>& FunctionGraph::getReferencesFrom() const {
    return m_referencedGraphsFrom;
}

void FunctionGraph::addReferenceFrom(InstructionOffset fromOffset, FunctionGraph* referencedGraph) {
    m_referencedGraphsFrom[fromOffset] = referencedGraph;
    referencedGraph->m_referencedGraphsTo.push_back(this);
}

void FunctionGraph::addReferenceFrom(InstructionOffset fromOffset, Block* calledBlock) {
    if (calledBlock->isEntryBlock()) {
        addReferenceFrom(fromOffset, calledBlock->getFunctionGraph());
    } else {
        auto graph = calledBlock->getGraph();
        auto prevUpdateBlockEnabled = graph->m_updateBlockEnabled;
        graph->m_updateBlockEnabled = false;
        auto newFuncGraph = graph->createFunctionGraph(calledBlock);
        addReferenceFrom(fromOffset, newFuncGraph);
        graph->m_updateBlockEnabled = prevUpdateBlockEnabled;
        calledBlock->update();
    }
}

void FunctionGraph::removeAllReferences() {
    moveReferences(nullptr);
}

void FunctionGraph::moveReferences(FunctionGraph* destGraph, InstructionOffset startFromOffset, InstructionOffset endFromOffset) {
    for (auto it = m_referencedGraphsFrom.begin(); it != m_referencedGraphsFrom.end();) {
        if (startFromOffset == InvalidOffset || it->first >= startFromOffset && it->first < endFromOffset) {
            it->second->m_referencedGraphsTo.remove(this);
            if (destGraph) {
                destGraph->addReferenceFrom(it->first, it->second);
            }
            it = m_referencedGraphsFrom.erase(it);
        } else {
            ++it;
        }
    }
}

std::list<Block*> FunctionGraph::toBlocks(const utils::BitSet& blockSet) const {
    std::list<Block*> blocks;
    for (const auto& [index, block] : m_indexToBlock) {
        // TODO: we could reduce the number of iterations by passing bitset instead of m_indexToBlock
        if (blockSet.get(index)) {
            blocks.push_back(block);
        }
    }
    return blocks;
}
