#include "Core/Symbol/FunctionSymbol.h"
#include "Core/SymbolTable/StandartSymbolTable.h"

using namespace sda;

FunctionSymbol::FunctionSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    SignatureDataType* dataType,
    SymbolTable* stackSymbolTable,
    SymbolTable* instructionSymbolTable)
    : Symbol(context, id, name, dataType)
    , m_stackSymbolTable(stackSymbolTable)
    , m_instructionSymbolTable(instructionSymbolTable)
{
    m_context->getSymbols()->add(std::unique_ptr<FunctionSymbol>(this));
}

SignatureDataType* FunctionSymbol::getSignature() const {
    return dynamic_cast<SignatureDataType*>(getDataType());
}

SymbolTable* FunctionSymbol::getStackSymbolTable() const {
    return m_stackSymbolTable;
}

SymbolTable* FunctionSymbol::getInstructionSymbolTable() const {
    return m_instructionSymbolTable;
}

void FunctionSymbol::serialize(boost::json::object& data) const {
    Symbol::serialize(data);
    data["type"] = Type;
    data["stack_symbol_table"] = m_stackSymbolTable->serializeId();
    data["instruction_symbol_table"] = m_instructionSymbolTable->serializeId();
}

void FunctionSymbol::deserialize(boost::json::object& data) {
    Symbol::deserialize(data);
    m_stackSymbolTable = m_context->getSymbolTables()->get(data["stack_symbol_table"]);
    m_instructionSymbolTable = m_context->getSymbolTables()->get(data["instruction_symbol_table"]);
}