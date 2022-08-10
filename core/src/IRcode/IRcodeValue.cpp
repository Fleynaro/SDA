#include "Core/IRcode/IRcodeValue.h"
#include "Core/DataType/PointerDataType.h"
#include "Core/DataType/StructureDataType.h"

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

void Value::setDataType(DataType* dataType) {
    m_dataType = dataType;
    m_symbolTable = nullptr;
    if (auto pointerDt = dynamic_cast<PointerDataType*>(dataType))
        if (auto structDt = dynamic_cast<StructureDataType*>(pointerDt->getPointedType()))
            m_symbolTable = structDt->getSymbolTable();
}

DataType* Value::getDataType() {
    return m_dataType;
}

void Value::setSymbolTable(SymbolTable* symbolTable) {
    m_symbolTable = symbolTable;
}

SymbolTable* Value::getSymbolTable() {
    return m_symbolTable;
}

void Value::setSymbol(Symbol* symbol) {
    m_symbol = symbol;
}

Symbol* Value::getSymbol() {
    return m_symbol;
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