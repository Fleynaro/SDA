#include "Core/DataType/EnumDataType.h"

using namespace sda;

EnumDataType::EnumDataType(Context* context, Object::Id* id, const std::string& name)
    : DataType(context, id, name)
{
    m_context->getDataTypes()->add(std::unique_ptr<EnumDataType>(this));
}

void EnumDataType::setFields(const std::map<Key, std::string>& fields) {
    m_context->getCallbacks()->onObjectModified(this);
    m_fields = fields;
}

const std::map<EnumDataType::Key, std::string>& EnumDataType::getFields() const {
    return m_fields;
}

size_t EnumDataType::getSize() const {
    return sizeof(Key);
}

void EnumDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;

    // serialize all fields
    boost::json::array fields;
    for (const auto& [key, name] : m_fields) {
        boost::json::object field;
        field["key"] = key;
        field["name"] = name;
        fields.push_back(field);
    }
    data["fields"] = fields;
}

void EnumDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);

    // deserialize all fields
    const auto& fields = data["fields"].get_array();
    for (const auto& fieldVal : fields) {
        auto field = fieldVal.get_object();
        const auto key = static_cast<Key>(field["key"].get_int64());
        const auto name = field["name"].get_string().c_str();
        m_fields[key] = name;
    }
}