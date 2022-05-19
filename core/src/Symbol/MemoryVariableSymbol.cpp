#include "Core/Symbol/MemoryVariableSymbol.h"

using namespace sda;

MemoryVariableSymbol::MemoryVariableSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType,
    Offset offset)
    : Symbol(context, id, name, dataType), m_offset(offset)
{}

Offset MemoryVariableSymbol::getOffset() const {
    return m_offset;
}

void MemoryVariableSymbol::serialize(boost::json::object& data) const {
    Symbol::serialize(data);
    data["type"] = Type;
    data["offset"] = m_offset;
}

void MemoryVariableSymbol::deserialize(boost::json::object& data) {
    Symbol::deserialize(data);
    m_offset = data["offset"].get_uint64();
}