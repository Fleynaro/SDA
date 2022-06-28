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

Constant::Constant(pcode::ConstantVarnode* constVarnode, Hash hash)
    : Value(hash), m_constVarnode(constVarnode)
{}

pcode::ConstantVarnode* Constant::getConstVarnode() const {
    return m_constVarnode;
}

size_t Constant::getSize() const {
    return m_constVarnode->getSize();
}

Register::Register(pcode::RegisterVarnode* regVarnode, Hash hash)
    : Value(hash), m_regVarnode(regVarnode)
{}

pcode::RegisterVarnode* Register::getRegVarnode() const {
    return m_regVarnode;
}

size_t Register::getSize() const {
    return m_regVarnode->getSize();
}

Variable::Variable(const MemoryAddress& memAddress, Hash hash, size_t size)
    : Value(hash)
    , m_memAddress(memAddress)
    , m_size(size)
{}

const MemoryAddress& Variable::getMemAddress() const {
    return m_memAddress;
}

size_t Variable::getSize() const {
    return m_size;
}