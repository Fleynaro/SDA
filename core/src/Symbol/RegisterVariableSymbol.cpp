#include "Core/Symbol/RegisterVariableSymbol.h"

using namespace sda;

RegisterVariableSymbol::RegisterVariableSymbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType,
    const std::list<ComplexOffset>& offsets)
    : Symbol(context, id, name, dataType), m_offsets(offsets)
{}

const std::list<ComplexOffset>& RegisterVariableSymbol::getOffsets() const {
    return m_offsets;
}

void RegisterVariableSymbol::serialize(boost::json::object& data) const {
    Symbol::serialize(data);
    data["type"] = Type;

    // serialize the list of offsets
    auto offsets = boost::json::array();
    for (auto offset : m_offsets)
        offsets.push_back(size_t(offset));
    data["offsets"] = offsets;
}

void RegisterVariableSymbol::deserialize(boost::json::object& data) {
    Symbol::deserialize(data);

    // deserialize the list of offsets
    const auto& offsets = data["offsets"].get_array();
    for (const auto& offset : offsets)
        m_offsets.push_back(ComplexOffset(offset.get_uint64()));
}