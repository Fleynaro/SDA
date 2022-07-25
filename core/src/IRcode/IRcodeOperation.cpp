#include "Core/IRcode/IRcodeFunction.h"

using namespace sda;
using namespace sda::ircode;

Operation::Operation(
    OperationId id,
    std::shared_ptr<Variable> output)
    : m_id(id)
    , m_output(output)
{
    m_output->getOperations().push_back(this);
}

OperationId Operation::getId() const {
    return m_id;
}

size_t Operation::getSize() const {
    return m_output->getSize();
}

std::shared_ptr<Variable> Operation::getOutput() const {
    return m_output;
}

std::set<pcode::Instruction*>& Operation::getPcodeInstructions() {
    return m_pcodeInstructions;
}

std::set<std::shared_ptr<Variable>>& Operation::getOverwrittenVariables() {
    return m_overwrittenVariables;
}

UnaryOperation::UnaryOperation(
    OperationId id,
    std::shared_ptr<Value> input,
    std::shared_ptr<Variable> output)
    : Operation(id, output)
    , m_input(input)
{
    m_input->getOperations().push_back(this);
}

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
{
    m_input1->getOperations().push_back(this);
    m_input2->getOperations().push_back(this);
}

std::shared_ptr<Value> BinaryOperation::getInput1() const {
    return m_input1;
}

std::shared_ptr<Value> BinaryOperation::getInput2() const {
    return m_input2;
}

ExtractOperation::ExtractOperation(
    std::shared_ptr<Value> input,
    size_t offset,
    std::shared_ptr<Variable> output)
    : UnaryOperation(OperationId::EXTRACT, input, output)
    , m_offset(offset)
{}

size_t ExtractOperation::getOffset() const {
    return m_offset;
}

ConcatOperation ::ConcatOperation(
    std::shared_ptr<Value> input1,
    std::shared_ptr<Value> input2,
    size_t offset,
    std::shared_ptr<Variable> output)
    : BinaryOperation(OperationId::CONCAT, input1, input2, output)
    , m_offset(offset)
{}

size_t ConcatOperation::getOffset() const {
    return m_offset;
}