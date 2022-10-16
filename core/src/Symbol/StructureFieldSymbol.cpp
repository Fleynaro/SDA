#include "SDA/Core/Symbol/StructureFieldSymbol.h"

using namespace sda;

StructureFieldSymbol::StructureFieldSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType)
    : Symbol(context, id, name, dataType)
{
    m_context->getSymbols()->add(std::unique_ptr<StructureFieldSymbol>(this));
}

void StructureFieldSymbol::serialize(boost::json::object& data) const {
    Symbol::serialize(data);
    data["type"] = Type;
}

void StructureFieldSymbol::deserialize(boost::json::object& data) {
    Symbol::deserialize(data);
}