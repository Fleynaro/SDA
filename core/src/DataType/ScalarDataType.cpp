#include "Core/DataType/ScalarDataType.h"

using namespace sda;

ScalarDataType::ScalarDataType(
    Context* context,
    ObjectId* id,
    const std::string& name,
    bool isFloatingPoint,
    bool isSigned,
    size_t size)
    : DataType(context, id, name)
    , m_isFloatingPoint(isFloatingPoint)
    , m_isSigned(isSigned)
    , m_size(size)
{}

bool ScalarDataType::isFloatingPoint() const {
    return m_isFloatingPoint;
}

bool ScalarDataType::isSigned() const {
    return m_isSigned;
}

size_t ScalarDataType::getSize() const {
    return m_size;
}

void ScalarDataType::serialize(boost::json::object& data) const {
    DataType::serialize(data);
    data["type"] = ScalarDataType::Type;
    data["is_fp"] = m_isFloatingPoint;
    data["is_signed"] = m_isSigned;
    data["size"] = m_size;
}

void ScalarDataType::deserialize(boost::json::object& data) {
    DataType::deserialize(data);
    m_isFloatingPoint = data["is_fp"].get_bool();
    m_isSigned = data["is_signed"].get_bool();
    m_size = data["size"].get_uint64();
}