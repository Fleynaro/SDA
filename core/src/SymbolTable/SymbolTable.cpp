#include "Core/SymbolTable/SymbolTable.h"

using namespace sda;

SymbolTable::SymbolTable(Context* context, Object::Id* id, const std::string& name)
    : ContextObject(context, id, name)
{
    m_context->getSymbolTables()->add(std::unique_ptr<SymbolTable>(this));
}

void SymbolTable::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
    data["collection"] = Collection;
}

void SymbolTable::destroy() {
    m_context->getSymbolTables()->remove(this);
}