#include "SDA/Core/IRcode/IRcodeFunction.h"

using namespace sda;
using namespace sda::ircode;

Function::Function(pcode::FunctionGraph* functionGraph)
    : m_functionGraph(functionGraph)
{}

pcode::FunctionGraph* Function::getFunctionGraph() const {
    return m_functionGraph;
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
