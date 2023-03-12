#include "SDA/Core/IRcode/IRcodeProgram.h"

using namespace sda;
using namespace sda::ircode;

void Program::PcodeGraphCallbacks::onBlockUpdateRequestedImpl(pcode::Block* pcodeBlock) {
    m_pcodeBlocksToUpdate.insert(pcodeBlock);
}

void Program::PcodeGraphCallbacks::onBlockFunctionGraphChangedImpl(pcode::Block* pcodeBlock, pcode::FunctionGraph* oldFunctionGraph, pcode::FunctionGraph* newFunctionGraph) {
    if (oldFunctionGraph) {
        auto oldFunction = m_program->toFunction(oldFunctionGraph);
        auto block = oldFunction->toBlock(pcodeBlock);
        oldFunction->getBlocks().erase(pcodeBlock);
    }
    auto newFunction = m_program->toFunction(newFunctionGraph);
    newFunction->getBlocks().emplace(pcodeBlock, Block(pcodeBlock, newFunction));
}

void Program::PcodeGraphCallbacks::onFunctionGraphCreatedImpl(pcode::FunctionGraph* functionGraph) {
    m_program->m_functions.emplace(functionGraph, Function(functionGraph));
}

void Program::PcodeGraphCallbacks::onFunctionGraphRemovedImpl(pcode::FunctionGraph* functionGraph) {
    m_program->m_functions.erase(functionGraph);
}

void Program::PcodeGraphCallbacks::onCommitStartedImpl() {
    m_commitStarted = true;
}

void Program::PcodeGraphCallbacks::onCommitEndedImpl() {
    m_commitStarted = false;
    updateBlocks();
}

void Program::PcodeGraphCallbacks::updateBlocks() {
    std::map<pcode::FunctionGraph*, std::list<pcode::Block*>> blocksToUpdateByFunction;
    for (auto pcodeBlock : m_pcodeBlocksToUpdate) {
        auto pcodeFunctionGraph = pcodeBlock->getFunctionGraph();
        blocksToUpdateByFunction[pcodeFunctionGraph].push_back(pcodeBlock);
    }
    for (auto& [pcodeFunctionGraph, pcodeBlocksToUpdate] : blocksToUpdateByFunction) {
        auto function = m_program->toFunction(pcodeFunctionGraph);
        utils::BitSet mutualDomBlockSet;
        for (auto pcodeBlock : pcodeBlocksToUpdate) {
            if (pcodeBlock == *pcodeBlocksToUpdate.begin()) {
                mutualDomBlockSet = pcodeBlock->getDominantBlocksSet();
            } else {
                mutualDomBlockSet = mutualDomBlockSet & pcodeBlock->getDominantBlocksSet();
            }
        }
        auto mutualDomBlocks = pcodeFunctionGraph->toBlocks(mutualDomBlockSet);
        pcode::Block* mostDominatedBlock = nullptr;
        size_t maxSize = 0;
        for (auto pcodeBlock : mutualDomBlocks) {
            auto size = pcodeBlock->getDominantBlocks().size();
            if (size > maxSize) {
                mostDominatedBlock = pcodeBlock;
                maxSize = size;
            }
        }
        assert(mostDominatedBlock);
        auto block = function->toBlock(mostDominatedBlock);
        block->update();
    }
    m_pcodeBlocksToUpdate.clear();
}

Program::Program(pcode::Graph* graph)
    : m_graph(graph), m_pcodeGraphCallbacks(std::make_shared<PcodeGraphCallbacks>(this))
{
    auto prevCallbacks = m_graph->getCallbacks();
    m_pcodeGraphCallbacks->setPrevCallbacks(prevCallbacks);
    m_graph->setCallbacks(m_pcodeGraphCallbacks);
}

pcode::Graph* Program::getGraph() {
    return m_graph;
}

std::map<pcode::FunctionGraph*, Function>& Program::getFunctions() {
    return m_functions;
}

Function* Program::toFunction(pcode::FunctionGraph* functionGraph) {
    auto it = m_functions.find(functionGraph);
    if (it == m_functions.end()) {
        return nullptr;
    }
    return &it->second;
}