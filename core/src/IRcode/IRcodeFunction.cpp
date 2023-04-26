#include "SDA/Core/IRcode/IRcodeFunction.h"

using namespace sda;
using namespace sda::ircode;

Function::Function(Program* program, pcode::FunctionGraph* functionGraph)
    : m_program(program), m_functionGraph(functionGraph)
{}

Program* Function::getProgram() const {
    return m_program;
}

pcode::FunctionGraph* Function::getFunctionGraph() const {
    return m_functionGraph;
}

Offset Function::getEntryOffset() const {
    return m_functionGraph->getEntryBlock()->getMinOffset();
}

Block* Function::getEntryBlock() {
    return toBlock(m_functionGraph->getEntryBlock());
}

std::map<pcode::Block*, Block>& Function::getBlocks() {
    return m_blocks;
}

Block* Function::toBlock(pcode::Block* pcodeBlock) {
    auto it = m_blocks.find(pcodeBlock);
    if (it == m_blocks.end()) {
        throw std::runtime_error("Block not found");
    }
    return &it->second;
}
