#include "SDA/Core/IRcode/IRcodeProgram.h"

using namespace sda;
using namespace sda::ircode;

void Program::PcodeGraphCallbacks::onBlockUpdatedImpl(pcode::Block* pcodeBlock) {
    auto pcodeFunctionGraph = pcodeBlock->getFunctionGraph();
    auto function = m_program->toFunction(pcodeFunctionGraph);
    auto block = function->toBlock(pcodeBlock);
    block->update();
    // todo: optimize
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
    // for optimization
}

void Program::PcodeGraphCallbacks::onCommitEndedImpl() {
    // for optimization
}

Program::Program(pcode::Graph* graph)
    : m_graph(graph), m_pcodeGraphCallbacks(this)
{}

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