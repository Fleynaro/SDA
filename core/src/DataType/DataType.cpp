#include "Core/DataType/DataType.h"
#include "Core/DataType/PointerDataType.h"
#include "Core/DataType/ArrayDataType.h"
#include "Core/DataType/ScalarDataType.h"
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
    data["collection"] = Collection;
}

void DataType::destroy() {
    m_context->getDataTypes()->remove(this);
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