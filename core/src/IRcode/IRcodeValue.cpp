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

const std::list<Operation*>& Value::getOperations() const {
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

Constant::Constant(const pcode::ConstantVarnode* constVarnode, Hash hash)
    : Value(hash), m_constVarnode(constVarnode)
{}

Value::Type Constant::getType() const {
    return Type::Constant;
}

const pcode::ConstantVarnode* Constant::getConstVarnode() const {
    return m_constVarnode;
}

size_t Constant::getSize() const {
    return m_constVarnode->getSize();
}

ircode::Register::Register(const pcode::RegisterVarnode* regVarnode, Hash hash)
    : Value(hash), m_regVarnode(regVarnode)
{}

Value::Type ircode::Register::getType() const {
    return Type::Register;
}

const pcode::RegisterVarnode* ircode::Register::getRegVarnode() const {
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

std::string Variable::getName() const {
    return std::string("var") + std::to_string(m_id);
}

Value::Type Variable::getType() const {
    return Type::Variable;
}

const MemoryAddress& Variable::getMemAddress() const {
    return m_memAddress;
}

size_t Variable::getSize() const {
    return m_size;
}