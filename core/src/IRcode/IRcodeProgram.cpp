#include "SDA/Core/IRcode/IRcodeProgram.h"

using namespace sda;
using namespace sda::ircode;

void Program::PcodeGraphCallbacks::onBlockUpdatedImpl(pcode::Block* block) {
    
}

void Program::PcodeGraphCallbacks::onFunctionGraphCreatedImpl(pcode::FunctionGraph* functionGraph) {
    m_program->m_functions.emplace(functionGraph, functionGraph);
}

void Program::PcodeGraphCallbacks::onFunctionGraphRemovedImpl(pcode::FunctionGraph* functionGraph) {
    m_program->m_functions.erase(functionGraph);
}

Program::Program(pcode::Graph* graph)
    : m_graph(graph), m_pcodeGraphCallbacks(this)
{}

std::map<pcode::FunctionGraph*, Function>& Program::getFunctions() {
    return m_functions;
}