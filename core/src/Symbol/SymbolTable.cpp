#include "Core/Symbol/SymbolTable.h"

using namespace sda;

SymbolTable::SymbolTable(Context* context, ObjectId* id, const std::string& name)
    : ContextObject(context, id, name)
{
    m_context->getSymbolTables()->add(std::unique_ptr<SymbolTable>(this));
}

void SymbolTable::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
}

void SymbolTable::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);
}

void SymbolTable::destroy() {
    m_context->getSymbolTables()->remove(getId());
}