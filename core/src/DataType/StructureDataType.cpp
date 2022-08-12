#include "Core/DataType/StructureDataType.h"
#include "Core/SymbolTable/StandartSymbolTable.h"
#include "Core/Context.h"

using namespace sda;

StructureDataType::StructureDataType(
    Context* context,
    Object::Id* id,
    const std::string& name,
    bool symbolTable)
    : DataType(context, id, name)
{
    if (symbolTable)
        m_symbolTable = new StandartSymbolTable(context, nullptr, "structure");
    m_context->getDataTypes()->add(std::unique_ptr<StructureDataType>(this));
}

StandartSymbolTable* StructureDataType::getSymbolTable() const {
    return m_symbolTable;
}

void StructureDataType::setSize(size_t size) {
    m_context->getCallbacks()->onObjectModified(this);
    m_size = size;
}

size_t StructureDataType::getSize() const {
    return m_size;
}

void StructureDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;
    data["size"] = m_size;
    data["symbol_table"] = m_symbolTable->serializeId();
}

void StructureDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
    m_size = data["size"].get_uint64();
    m_symbolTable = dynamic_cast<StandartSymbolTable*>(m_context->getSymbolTables()->get(data["symbol_table"]));
    if (!m_symbolTable)
        throw std::runtime_error("StructureDataType::deserialize: symbol table is null");
}