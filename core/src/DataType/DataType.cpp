#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/DataType/VoidDataType.h"
#include "SDA/Core/DataType/PointerDataType.h"
#include "SDA/Core/DataType/ArrayDataType.h"
#include "SDA/Core/DataType/ScalarDataType.h"
#include "SDA/Core/DataType/DataTypeParser.h"
#include <boost/functional/hash.hpp>

using namespace sda;

DataType::DataType(Context* context, Object::Id* id, const std::string& name)
    : ContextObject(context, id, name)
{}

void DataType::notifyModified(Object::ModState state) {
    ContextObject::notifyModified(state);
    m_context->getDataTypes()->notifyModified(this, state);
}

PointerDataType* DataType::getPointerTo() {
    auto dataTypeName = PointerDataType::GetTypeName(this);
    if (auto dataType = m_context->getDataTypes()->getByName(dataTypeName))
        return dynamic_cast<PointerDataType*>(dataType);
    return new PointerDataType(m_context, nullptr, this);
}

ArrayDataType* DataType::getArrayOf(const std::list<size_t>& dimensions) {
    auto dataTypeName = ArrayDataType::GetTypeName(this, dimensions);
    if (auto dataType = m_context->getDataTypes()->getByName(dataTypeName))
        return dynamic_cast<ArrayDataType*>(dataType);
    return new ArrayDataType(m_context, nullptr, this, dimensions);
}

DataType* DataType::getBaseType() {
    return this;
}

bool DataType::isVoid() const {
    return false;
}

bool DataType::isPointer() const {
    return false;
}

bool DataType::isScalar(ScalarType type) const {
    return false;
}

void DataType::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
    data["class"] = Class;
}

void DataType::destroy() {
    m_context->getDataTypes()->remove(this);
}

void DataTypeList::initDefault() {
    auto ctx = getContext();
    new VoidDataType(ctx, nullptr);
    new ScalarDataType(ctx, nullptr, "uint8_t", ScalarType::UnsignedInt, 1);
    new ScalarDataType(ctx, nullptr, "int8_t", ScalarType::SignedInt, 1);
    new ScalarDataType(ctx, nullptr, "uint16_t", ScalarType::UnsignedInt, 2);
    new ScalarDataType(ctx, nullptr, "int16_t", ScalarType::SignedInt, 2);
    new ScalarDataType(ctx, nullptr, "uint32_t", ScalarType::UnsignedInt, 4);
    new ScalarDataType(ctx, nullptr, "int32_t", ScalarType::SignedInt, 4);
    new ScalarDataType(ctx, nullptr, "uint64_t", ScalarType::UnsignedInt, 8);
    new ScalarDataType(ctx, nullptr, "int64_t", ScalarType::SignedInt, 8);
    new ScalarDataType(ctx, nullptr, "float", ScalarType::FloatingPoint, 4);
    new ScalarDataType(ctx, nullptr, "double", ScalarType::FloatingPoint, 8);

    // derived data types
    auto dataTypesStr = "\
        bool = typedef uint8_t \
    ";
    DataTypeParser::Parse(dataTypesStr, ctx);
}

DataType* DataTypeList::getByName(const std::string& name) const {
    auto it = m_dataTypes.find(name);
    if (it != m_dataTypes.end()) {
        return it->second;
    }
    return nullptr;
}

ScalarDataType* DataTypeList::getScalar(ScalarType type, size_t size) {
    auto it = m_scalarDataTypes.find(scalarToHash(type, size));
    if (it != m_scalarDataTypes.end()) {
        return it->second;
    }
    throw std::runtime_error("Scalar data type not found");
}

void DataTypeList::notifyModified(DataType* dataType, Object::ModState state) {
    if (state == Object::ModState::Before) {
        onObjectRemoved(dataType);
    } else if (state == Object::ModState::After) {
        onObjectAdded(dataType);
    }
}

void DataTypeList::onObjectAdded(DataType* dataType) {
    m_dataTypes[dataType->getName()] = dataType;
    if (auto scalarDt = dynamic_cast<ScalarDataType*>(dataType)) {
        auto hash = scalarToHash(scalarDt->getScalarType(), scalarDt->getSize());
        m_scalarDataTypes[hash] = scalarDt;
    }
}

void DataTypeList::onObjectRemoved(DataType* dataType) {
    m_dataTypes.erase(dataType->getName());
    if (auto scalarDt = dynamic_cast<ScalarDataType*>(dataType)) {
        auto hash = scalarToHash(scalarDt->getScalarType(), scalarDt->getSize());
        m_scalarDataTypes.erase(hash);
    }
}

size_t DataTypeList::scalarToHash(ScalarType type, size_t size) const {
    size_t hash = 0;
    boost::hash_combine(hash, type);
    boost::hash_combine(hash, size);
    return hash;
}