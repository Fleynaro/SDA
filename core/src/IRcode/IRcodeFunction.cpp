#include "SDA/Core/IRcode/IRcodeFunction.h"
#include "SDA/Core/IRcode/IRcodeProgram.h"

using namespace sda;
using namespace sda::ircode;

Function::Function(Program* program, pcode::FunctionGraph* functionGraph)
    : m_program(program), m_functionGraph(functionGraph)
{}

std::string Function::getName() {
    return getEntryBlock()->getName();
}

Program* Function::getProgram() const {
    return m_program;
}

pcode::FunctionGraph* Function::getFunctionGraph() const {
    return m_functionGraph;
}

FunctionSymbol* Function::getFunctionSymbol() const {
    auto offset = getEntryOffset();
    auto globalSymbolTable = m_program->getGlobalSymbolTable();
    auto symbol = globalSymbolTable->getSymbolAt(offset).symbol;
    return dynamic_cast<FunctionSymbol*>(symbol);
}

Offset Function::getEntryOffset() const {
    return m_functionGraph->getEntryOffset();
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

std::list<std::shared_ptr<Variable>> Function::getVariables() {
    std::list<std::shared_ptr<Variable>> variables;
    for (auto& [pcodeBlock, ircodeBlock] : m_blocks) {
        for (auto& op : ircodeBlock.getOperations()) {
            variables.push_back(op->getOutput());
        }
    }
    return variables;
}

const std::vector<std::shared_ptr<Variable>>& Function::getParamVariables() {
    return m_paramVars;
}

std::shared_ptr<Variable> Function::getReturnVariable() {
    return m_returnVar;
}

std::shared_ptr<Variable> Function::findVariableById(size_t id) {
    if (m_varIds.get(id)) {
        for (auto var : getVariables()) {
            if (var->getId() == id) {
                return var;
            }
        }
    }
    return nullptr;
}
