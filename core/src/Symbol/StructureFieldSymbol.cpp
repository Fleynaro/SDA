#include "Core/Symbol/StructureFieldSymbol.h"

using namespace sda;

StructureFieldSymbol::StructureFieldSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType,
    Offset offset)
    : Symbol(context, id, name, dataType), m_offset(offset)
{}

Offset StructureFieldSymbol::getOffset() const {
    return m_offset;
}

void StructureFieldSymbol::serialize(boost::json::object& data) const {
    Symbol::serialize(data);
    data["type"] = Type;
    data["offset"] = m_offset;
}

void StructureFieldSymbol::deserialize(boost::json::object& data) {
    Symbol::deserialize(data);
    m_offset = data["offset"].get_uint64();
}