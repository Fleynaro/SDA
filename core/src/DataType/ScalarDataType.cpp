#include "SDA/Core/DataType/ScalarDataType.h"

using namespace sda;

ScalarDataType::ScalarDataType(
    Context* context,
    Object::Id* id,
    const std::string& name,
    ScalarType scalarType,
    size_t size)
    : DataType(context, id, name)
    , m_scalarType(scalarType)
    , m_size(size)
{
    m_context->getDataTypes()->add(std::unique_ptr<ScalarDataType>(this));
}

ScalarType ScalarDataType::getScalarType() const {
    return m_scalarType;
}

bool ScalarDataType::isScalar(ScalarType type) const {
    return m_scalarType == type;
}

size_t ScalarDataType::getSize() const {
    return m_size;
}

void ScalarDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = Type;
    data["scalar_type"] = static_cast<size_t>(m_scalarType);
    data["size"] = m_size;
}

void ScalarDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
    m_scalarType = utils::get_number<ScalarType>(data["scalar_type"]);
    m_size = utils::get_number<size_t>(data["size"]);
    notifyModified(Object::ModState::After);
}