#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/Pcode/PcodeGraph.h"

using namespace sda;
using namespace sda::ircode;

void Program::PcodeGraphCallbacks::onBlockUpdateRequestedImpl(pcode::Block* pcodeBlock) {
    m_pcodeBlocksToUpdate.insert(pcodeBlock);
}

void Program::PcodeGraphCallbacks::onBlockFunctionGraphChangedImpl(pcode::Block* pcodeBlock, pcode::FunctionGraph* oldFunctionGraph, pcode::FunctionGraph* newFunctionGraph) {
    if (oldFunctionGraph) {
        auto oldFunction = m_program->toFunction(oldFunctionGraph);
        auto block = oldFunction->toBlock(pcodeBlock);
        m_program->getCallbacks()->onBlockRemoved(block);
        oldFunction->getBlocks().erase(pcodeBlock);
    }
    auto newFunction = m_program->toFunction(newFunctionGraph);
    newFunction->getBlocks().emplace(pcodeBlock, Block(pcodeBlock, newFunction));
    m_program->getCallbacks()->onBlockCreated(newFunction->toBlock(pcodeBlock));
}

void Program::PcodeGraphCallbacks::onFunctionGraphCreatedImpl(pcode::FunctionGraph* functionGraph) {
    m_program->m_functions.emplace(functionGraph, Function(m_program, functionGraph));
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

Program::Program(pcode::Graph* graph, SymbolTable* globalSymbolTable)
    : m_graph(graph)
    , m_globalSymbolTable(globalSymbolTable)
    , m_pcodeGraphCallbacks(std::make_shared<PcodeGraphCallbacks>(this))
    , m_callbacks(std::make_shared<Callbacks>())
{
    if (m_graph) {
        auto prevCallbacks = m_graph->getCallbacks();
        m_pcodeGraphCallbacks->setPrevCallbacks(prevCallbacks);
        m_graph->setCallbacks(m_pcodeGraphCallbacks);
    }
}

pcode::Graph* Program::getGraph() {
    return m_graph;
}

SymbolTable* Program::getGlobalSymbolTable() {
    return m_globalSymbolTable;
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

Function* Program::getFunctionAt(pcode::InstructionOffset offset) {
    auto functionGraph = m_graph->getFunctionGraphAt(offset);
    if (functionGraph) {
        return toFunction(functionGraph);
    }
    return nullptr;
}

std::list<Function*> Program::getFunctionsByCallInstruction(const pcode::Instruction* instr) {
    auto offset = GetTargetOffset(instr);
    if (offset != InvalidOffset) {
        auto function = getFunctionAt(offset);
        if (function) {
            return { function };
        }
        return {};
    }
    // TODO: virtual calls
    return {};
}

void Program::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<Program::Callbacks> Program::getCallbacks() const {
    return m_callbacks;
}
