#include "Core/Symbol/FunctionSymbol.h"
#include "Core/SymbolTable/StandartSymbolTable.h"

using namespace sda;

FunctionSymbol::FunctionSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType,
    bool stackSymbolTable,
    bool instructionSymbolTable)
    : Symbol(context, id, name, dataType)
{
    if (stackSymbolTable)
        m_stackSymbolTable = new StandartSymbolTable(context, nullptr, "stack");
    if (instructionSymbolTable)
        m_instructionSymbolTable = new StandartSymbolTable(context, nullptr, "instruction");
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