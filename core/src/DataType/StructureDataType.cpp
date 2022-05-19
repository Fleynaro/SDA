#include "Core/DataType/StructureDataType.h"
#include "Core/Context.h"

using namespace sda;

StructureDataType::StructureDataType(Context* context, Object::Id* id, const std::string& name)
    : DataType(context, id, name)
{}

void StructureDataType::setFields(const std::map<Offset, StructureFieldSymbol*>& fields) {
    m_context->getCallbacks()->onObjectModified(this);
    m_fields = fields;
}

const std::map<Offset, StructureFieldSymbol*>& StructureDataType::getFields() const {
    return m_fields;
}

void StructureDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;

    // serialize all fields
    boost::json::array fields;
    for (const auto& [offset, field] : m_fields) {
        boost::json::object fieldData;
        fieldData["offset"] = offset;
        fieldData["field"] = field->serializeId();
    }
    data["fields"] = fields;
}

void StructureDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);

    // deserialize all fields
    m_fields.clear();
    const auto& fields = data["fields"].get_array();
    for (auto fieldData : fields) {
        auto offset = fieldData.get_object()["offset"].get_uint64();
        auto symbol = m_context->getSymbols()->get(fieldData.get_object()["field"]);
        if (auto structureFieldSymbol = dynamic_cast<StructureFieldSymbol*>(symbol))
            m_fields[offset] = structureFieldSymbol;
    }
}