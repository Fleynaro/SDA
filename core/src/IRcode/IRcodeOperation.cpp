#include "Core/IRcode/IRcodeFunction.h"

using namespace sda;
using namespace sda::ircode;

Operation::Operation(OperationId operationId)
    : m_id(operationId)
{}

OperationId Operation::getId() const {
    return m_id;
}

const std::vector<std::shared_ptr<Value>>& Operation::getInputs() const {
    return m_inputs;
}

std::shared_ptr<Variable> Operation::getOutput() const {
    return m_output;
}

const std::set<pcode::Instruction>& Operation::getPcodeInstructions() const {
    return m_pcodeInstructions;
}