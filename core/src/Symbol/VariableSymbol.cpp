#include "Core/Symbol/VariableSymbol.h"

using namespace sda;

VariableSymbol::VariableSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType)
    : Symbol(context, id, name, dataType)
{}

void VariableSymbol::serialize(boost::json::object& data) const {
    Symbol::serialize(data);
    data["type"] = Type;
}

void VariableSymbol::deserialize(boost::json::object& data) {
    Symbol::deserialize(data);
}