#include "SDA/Core/IRcode/IRcodeFunction.h"

using namespace sda;
using namespace sda::ircode;

Operation::Operation(
    OperationId id,
    std::shared_ptr<Variable> output)
    : m_id(id)
    , m_output(output)
{
    m_output->addOperation(this);
    if (auto memAddrValue = m_output->getMemAddress().value) {
        memAddrValue->addOperation(this);
    }
}

Operation::~Operation() {
    m_output->removeOperation(this);
    if (auto memAddrValue = m_output->getMemAddress().value) {
        memAddrValue->removeOperation(this);
    }
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

Block* Operation::getBlock() const {
    return m_block;
}

void Operation::setBlock(Block* block) {
    m_block = block;
}

const std::set<const pcode::Instruction*>& Operation::getPcodeInstructions() const {
    return m_pcodeInstructions;
}

void Operation::addPcodeInstruction(const pcode::Instruction* instruction) {
    m_pcodeInstructions.insert(instruction);
}

const std::set<std::shared_ptr<Variable>>& Operation::getOverwrittenVariables() const {
    return m_overwrittenVariables;
}

void Operation::setOverwrittenVariables(const std::set<std::shared_ptr<Variable>>& variables) {
    m_overwrittenVariables = variables;
}

UnaryOperation::UnaryOperation(
    OperationId id,
    std::shared_ptr<Value> input,
    std::shared_ptr<Variable> output)
    : Operation(id, output)
    , m_input(input)
{
    m_input->addOperation(this);
}

UnaryOperation::~UnaryOperation() {
    m_input->removeOperation(this);
}

Hash UnaryOperation::getHash() const {
    Hash hash = 0;
    boost::hash_combine(hash, getInput()->getHash());
    boost::hash_combine(hash, getOutput()->getHash());
    return hash;
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
    m_input1->addOperation(this);
    m_input2->addOperation(this);
}

BinaryOperation::~BinaryOperation() {
    m_input1->removeOperation(this);
    m_input2->removeOperation(this);
}

Hash BinaryOperation::getHash() const {
    Hash hash = 0;
    boost::hash_combine(hash, getInput1()->getHash());
    boost::hash_combine(hash, getInput2()->getHash());
    boost::hash_combine(hash, getOutput()->getHash());
    return hash;
}

std::shared_ptr<Value> BinaryOperation::getInput1() const {
    return m_input1;
}

std::shared_ptr<Value> BinaryOperation::getInput2() const {
    return m_input2;
}

CallOperation::CallOperation(
    std::shared_ptr<Value> dest,
	const std::vector<std::shared_ptr<Value>>& args,
    std::shared_ptr<Variable> output)
    : Operation(OperationId::CALL, output)
    , m_dest(dest)
    , m_args(args)
{
    m_dest->addOperation(this);
    for (auto arg : m_args) {
        arg->addOperation(this);
    }
}

CallOperation::~CallOperation() {
    m_dest->removeOperation(this);
    for (auto arg : m_args) {
        arg->removeOperation(this);
    }
}

Hash CallOperation::getHash() const {
    Hash hash = 0;
    boost::hash_combine(hash, m_dest->getHash());
    for (auto input : m_args) {
        boost::hash_combine(hash, input->getHash());
    }
    boost::hash_combine(hash, getOutput()->getHash());
    return hash;
}

std::shared_ptr<Value> CallOperation::getDestination() const {
    return m_dest;
}

const std::vector<std::shared_ptr<Value>>& CallOperation::getArguments() const {
    return m_args;
}

const pcode::Instruction* CallOperation::getPcodeInstruction() const {
    return *getPcodeInstructions().begin();
}

Hash RefOperation::Reference::getHash() const {
    Hash hash = 0;
    boost::hash_combine(hash, block);
    boost::hash_combine(hash, baseAddrHash);
    boost::hash_combine(hash, offset);
    boost::hash_combine(hash, size);
    return hash;
}

RefOperation::RefOperation(
    const RefOperation::Reference& reference,
    std::shared_ptr<Variable> input,
    std::shared_ptr<Variable> output)
    : UnaryOperation(OperationId::REF, input, output)
    , m_reference(reference)
{}

Hash RefOperation::getHash() const {
    return m_reference.getHash();
}

const RefOperation::Reference& RefOperation::getReference() const {
    return m_reference;
}

void RefOperation::setTargetVariable(std::shared_ptr<Variable> variable) {
    m_input = variable;
    variable->addOperation(this);
}

std::shared_ptr<Variable> RefOperation::getTargetVariable() const {
    return std::dynamic_pointer_cast<Variable>(m_input);
}

void RefOperation::clear() {
    m_input->removeOperation(this);
    m_input = nullptr;
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

ConcatOperation::ConcatOperation(
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

PhiOperation::PhiOperation(
    std::shared_ptr<Value> input1,
    std::shared_ptr<Value> input2,
    std::shared_ptr<Variable> output)
    : BinaryOperation(OperationId::PHI, input1, input2, output)
{}