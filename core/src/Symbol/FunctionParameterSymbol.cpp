#include "Core/Symbol/FunctionParameterSymbol.h"

using namespace sda;

FunctionParameterSymbol::FunctionParameterSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType)
    : Symbol(context, id, name, dataType)
{}

void FunctionParameterSymbol::serialize(boost::json::object& data) const {
    Symbol::serialize(data);
    data["type"] = Type;
}

void FunctionParameterSymbol::deserialize(boost::json::object& data) {
    Symbol::deserialize(data);
}