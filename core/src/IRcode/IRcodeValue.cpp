#include "SDA/Core/IRcode/IRcodeValue.h"
#include "SDA/Core/DataType/PointerDataType.h"
#include "SDA/Core/DataType/StructureDataType.h"

using namespace sda;
using namespace sda::ircode;

Value::Value(Hash hash)
    : m_hash(hash)
{}

Hash Value::getHash() const {
    return m_hash;
}

std::list<Operation*> Value::getOperations() const {
    return m_operations;
}

void Value::addOperation(Operation* operation) {
    m_operations.push_back(operation);
}

const LinearExpression& Value::getLinearExpr() const {
    return m_linearExpr;
}

void Value::setLinearExpr(const LinearExpression& linearExpr) {
    m_linearExpr = linearExpr;
}

Constant::Constant(std::shared_ptr<pcode::ConstantVarnode> constVarnode, Hash hash)
    : Value(hash), m_constVarnode(constVarnode)
{}

Value::Type Constant::getType() const {
    return Type::Constant;
}

std::shared_ptr<pcode::ConstantVarnode> Constant::getConstVarnode() const {
    return m_constVarnode;
}

size_t Constant::getSize() const {
    return m_constVarnode->getSize();
}

ircode::Register::Register(std::shared_ptr<pcode::RegisterVarnode> regVarnode, Hash hash)
    : Value(hash), m_regVarnode(regVarnode)
{}

Value::Type ircode::Register::getType() const {
    return Type::Register;
}

std::shared_ptr<pcode::RegisterVarnode> ircode::Register::getRegVarnode() const {
    return m_regVarnode;
}

const sda::Register& ircode::Register::getRegister() const {
    return m_regVarnode->getRegister();
}

size_t ircode::Register::getSize() const {
    return m_regVarnode->getSize();
}

Variable::Variable(size_t id, const MemoryAddress& memAddress, Hash hash, size_t size)
    : Value(hash)
    , m_id(id)
    , m_memAddress(memAddress)
    , m_size(size)
{}

size_t Variable::getId() const {
    return m_id;
}

std::string Variable::getName() const {
    return std::string("var") + std::to_string(m_id);
}

Value::Type Variable::getType() const {
    return Type::Variable;
}

Operation* Variable::getSourceOperation() const {
    return m_operations.empty() ? nullptr : m_operations.front();
}

const MemoryAddress& Variable::getMemAddress() const {
    return m_memAddress;
}

size_t Variable::getSize() const {
    return m_size;
}

RefVariable::RefVariable(std::shared_ptr<Variable> refVariable, const Reference& reference)
    : Variable(refVariable->getId(), refVariable->getMemAddress(), 0, refVariable->getSize())
    , m_reference(reference)
{
    setLinearExpr(refVariable->getLinearExpr());
    calcHash();
}

std::list<Operation*> RefVariable::getOperations() const {
    return getTargetVariable() ? getTargetVariable()->getOperations() : std::list<Operation*>();
}

const RefVariable::Reference& RefVariable::getReference() const {
    return m_reference;
}

void RefVariable::setTargetVariable(std::shared_ptr<Variable> variable) {
    m_targetVariable = variable;
    m_id = variable->getId();
}

std::shared_ptr<Variable> RefVariable::getTargetVariable() const {
    return m_targetVariable;
}

void RefVariable::calcHash() {
    boost::hash_combine(m_hash, m_reference.block);
    boost::hash_combine(m_hash, m_reference.baseAddrHash);
    boost::hash_combine(m_hash, m_reference.offset);
    boost::hash_combine(m_hash, m_reference.size);
}
