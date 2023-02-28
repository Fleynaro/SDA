#include "SDA/Core/Pcode/PcodeFunctionGraph.h"
#include "SDA/Core/Pcode/PcodeGraph.h"

using namespace sda::pcode;

FunctionGraph::FunctionGraph(Block* entryBlock)
    : m_entryBlock(entryBlock)
{}

Block* FunctionGraph::getEntryBlock() const {
    return m_entryBlock;
}

std::list<FunctionGraph::BlockInfo> FunctionGraph::getBlocks(bool sort) const {
    std::map<Block*, BlockInfo> blocks;
    // pass blocks
    m_entryBlock->passDescendants([&](Block* block, bool& goNextBlocks) {
        if (blocks.find(block) != blocks.end() || block->getEntryBlock() != m_entryBlock) {
            return;
        }
        size_t level = 1;
        for (auto refBlock : block->getReferencedBlocks()) {
            auto it = blocks.find(refBlock);
            if (it != blocks.end()) {
                level = std::max(level, it->second.level + 1);
            }
        }
        blocks[block] = { block, level };
        goNextBlocks = true;
    });
    // convert to list
    std::list<BlockInfo> result;
    for (auto& block : blocks) {
        result.push_back(block.second);
    }
    // sort blocks by level and offset
    if (sort) {
        result.sort([](const FunctionGraph::BlockInfo& a, const FunctionGraph::BlockInfo& b) {
            if (a.level != b.level)
                return a.level < b.level;
            return a.block->getMinOffset() < b.block->getMinOffset();
        });
    }
    return result;
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
        graph->setUpdateBlocksEnabled(false);
        auto newFuncGraph = graph->createFunctionGraph(calledBlock);
        addReferenceFrom(fromOffset, newFuncGraph);
        graph->setUpdateBlocksEnabled(true);
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
