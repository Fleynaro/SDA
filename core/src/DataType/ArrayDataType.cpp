#include "Core/DataType/ArrayDataType.h"

using namespace sda;

ArrayDataType::ArrayDataType(
    Context* context,
    ObjectId* id,
    DataType* elementType,
    const std::list<size_t>& dimensions)
    : DataType(context, id),
    m_elementType(elementType),
    m_dimensions(dimensions)
{
    if (elementType) {
        createName();
    }
}

DataType* ArrayDataType::getElementType() const {
    return m_elementType;
}

const std::list<size_t>& ArrayDataType::getDimensions() const {
    return m_dimensions;
}

size_t ArrayDataType::getSize() const {
    size_t size = m_elementType->getSize();
    for (auto dimension : m_dimensions)
        size *= dimension;
    return size;
}

void ArrayDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = ArrayDataType::Type;

    data["element_type"] = m_elementType->serializeId();

    boost::json::array dimensions;
    for (auto dimension : m_dimensions)
        dimensions.push_back(dimension);
    data["dimensions"] = dimensions;
}

void ArrayDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);

    m_elementType = m_context->getDataTypes()->get(data["element_type"]);

    m_dimensions.clear();
    const auto& dimensions = data["dimensions"].get_array();
    for (auto dimension : dimensions)
        m_dimensions.push_back(dimension.get_uint64());

    createName();
}

void ArrayDataType::createName() {
    std::string name = m_elementType->getName();
    for (auto dimension : m_dimensions)
        name += "[" + std::to_string(dimension) + "]";
    setName(name);
}