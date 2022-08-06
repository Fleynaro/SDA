#include "Core/Symbol/FunctionParameterSymbol.h"

using namespace sda;

FunctionParameterSymbol::FunctionParameterSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType)
    : Symbol(context, id, name, dataType)
{
    m_context->getSymbols()->add(std::unique_ptr<FunctionParameterSymbol>(this));
}

void FunctionParameterSymbol::serialize(boost::json::object& data) const {
    Symbol::serialize(data);
    data["type"] = Type;
}

void FunctionParameterSymbol::deserialize(boost::json::object& data) {
    Symbol::deserialize(data);
}