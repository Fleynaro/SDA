#include "Core/Symbol/VariableSymbol.h"

using namespace sda;

VariableSymbol::VariableSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType)
    : Symbol(context, id, name, dataType)
{
    m_context->getSymbols()->add(std::unique_ptr<VariableSymbol>(this));
}

void VariableSymbol::serialize(boost::json::object& data) const {
    Symbol::serialize(data);
    data["type"] = Type;
}

void VariableSymbol::deserialize(boost::json::object& data) {
    Symbol::deserialize(data);
}