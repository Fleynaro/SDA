#include "SDA/Core/Symbol/Symbol.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/DataType/DataType.h"

using namespace sda;

Symbol::Symbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType)
    : ContextObject(context, id, name)
    , m_dataType(dataType)
{
    if (m_dataType) {
        m_dataType->addParent(this);
    }
}

Symbol::~Symbol() {
    m_dataType->removeParent(this);
}

SymbolTable* Symbol::getSymbolTable() const {
    for (auto parent : getParents()) {
        if (auto symbolTable = dynamic_cast<SymbolTable*>(parent)) {
            return symbolTable;
        }
    }
    return nullptr;
}

Offset Symbol::getOffset() const {
    return m_offset;
}

void Symbol::setSymbolTable(SymbolTable* symbolTable, Offset offset) {
    if (getSymbolTable()) {
        throw std::runtime_error("Symbol already has a symbol table");
    }
    notifyModified(Object::ModState::Before);
    addParent(symbolTable);
    m_offset = offset;
    notifyModified(Object::ModState::After);
}

void Symbol::unsetSymbolTable() {
    if (!getSymbolTable()) {
        throw std::runtime_error("Symbol doesn't have a symbol table");
    }
    notifyModified(Object::ModState::Before);
    removeParent(getSymbolTable());
    m_offset = 0;
    notifyModified(Object::ModState::After);
}

DataType* Symbol::getDataType() const {
    return m_dataType;
}

void Symbol::setDataType(DataType* dataType) {
    notifyModified(Object::ModState::Before);
    m_dataType = dataType;
    notifyModified(Object::ModState::After);
}

void Symbol::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
    data["collection"] = Class;
    data["data_type"] = m_dataType->serializeId();
}

void Symbol::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);

    m_dataType = m_context->getDataTypes()->get(data["data_type"]);
    m_dataType->addParent(this);
}

void Symbol::destroy() {
    m_context->getSymbols()->remove(this);
}