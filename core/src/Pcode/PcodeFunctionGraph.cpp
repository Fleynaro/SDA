#include "SDA/Core/Pcode/PcodeFunctionGraph.h"
#include "SDA/Core/Pcode/PcodeGraph.h"

using namespace sda::pcode;

FunctionGraph::FunctionGraph(Block* entryBlock)
    : m_entryBlock(entryBlock)
{}

Block* FunctionGraph::getEntryBlock() const {
    return m_entryBlock;
}

std::list<Block*> FunctionGraph::getBlocks() const {
    std::list<Block*> blocks;
    std::list<Block*> toVisit;
    toVisit.push_back(m_entryBlock);
    while (!toVisit.empty()) {
        auto block = toVisit.front();
        toVisit.pop_front();
        if (std::find(blocks.begin(), blocks.end(), block) != blocks.end()) {
            continue;
        }
        blocks.push_back(block);
        for (auto nextBlock : block->getNextBlocks()) {
            if (m_entryBlock != nextBlock->getEntryBlock()) {
                continue;
            }
            if (!block->hasLoopWith(nextBlock)) {
                toVisit.push_back(nextBlock);
            }
        }
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
