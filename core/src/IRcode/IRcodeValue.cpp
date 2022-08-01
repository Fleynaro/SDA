#include "Core/IRcode/IRcodeValue.h"

using namespace sda;
using namespace sda::ircode;

Value::Value(Hash hash)
    : m_hash(hash)
{}

Hash Value::getHash() const {
    return m_hash;
}

std::list<Operation*>& Value::getOperations() {
    return m_operations;
}

LinearExpression& Value::getLinearExpr() {
    return m_linearExpr;
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

Register::Register(const pcode::RegisterVarnode* regVarnode, Hash hash)
    : Value(hash), m_regVarnode(regVarnode)
{}

Value::Type Register::getType() const {
    return Type::Register;
}

const pcode::RegisterVarnode* Register::getRegVarnode() const {
    return m_regVarnode;
}

size_t Register::getSize() const {
    return m_regVarnode->getSize();
}

bool MemoryAddress::isDynamic() const {
    return value->getLinearExpr().getTerms().size() > 1;
}

Variable::Variable(size_t id, const MemoryAddress& memAddress, Hash hash, size_t size)
    : Value(hash)
    , m_id(id)
    , m_memAddress(memAddress)
    , m_size(size)
{}

Value::Type Variable::getType() const {
    return Type::Variable;
}

size_t Variable::getId() const {
    return m_id;
}

const MemoryAddress& Variable::getMemAddress() const {
    return m_memAddress;
}

size_t Variable::getSize() const {
    return m_size;
}