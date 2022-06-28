#include "Core/IRcode/IRcodeFunction.h"

using namespace sda;
using namespace sda::ircode;

Operation::Operation(
    OperationId id,
    std::shared_ptr<Variable> output)
    : m_id(id)
    , m_output(output)
{}

OperationId Operation::getId() const {
    return m_id;
}

std::shared_ptr<Variable> Operation::getOutput() const {
    return m_output;
}

const std::set<pcode::Instruction>& Operation::getPcodeInstructions() const {
    return m_pcodeInstructions;
}

UnaryOperation::UnaryOperation(
    OperationId id,
    std::shared_ptr<Value> input,
    std::shared_ptr<Variable> output)
    : Operation(id, output)
    , m_input(input)
{}

std::shared_ptr<Value> UnaryOperation::getInput() const {
    return m_input;
}

BinaryOperation::BinaryOperation(
    OperationId id,
    std::shared_ptr<Value> input1,
    std::shared_ptr<Value> input2,
    std::shared_ptr<Variable> output)
    : Operation(id, output)
    , m_input1(input1)
    , m_input2(input2)
{}

std::shared_ptr<Value> BinaryOperation::getInput1() const {
    return m_input1;
}

std::shared_ptr<Value> BinaryOperation::getInput2() const {
    return m_input2;
}

ExtractOperation::ExtractOperation(
    std::shared_ptr<Value> input,
    size_t offset,
    size_t size,
    std::shared_ptr<Variable> output)
    : UnaryOperation(OperationId::EXTRACT, input, output)
    , m_offset(offset)
    , m_size(size)
{}