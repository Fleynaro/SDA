#include "SDA/Core/IRcode/IRcodeFunction.h"

using namespace sda::ircode;

Function::Function(pcode::FunctionGraph* functionGraph)
    : m_functionGraph(functionGraph)
{}

std::list<Block>& Function::getBlocks() {
    return m_blocks;
}