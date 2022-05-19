#include "Core/DataType/DataType.h"
#include "Core/DataType/ScalarDataType.h"

using namespace sda;

DataType::DataType(Context* context, Object::Id* id, const std::string& name)
    : ContextObject(context, id, name)
{
    m_context->getDataTypes()->add(std::unique_ptr<DataType>(this));
}

void DataType::destroy() {
    m_context->getDataTypes()->remove(this);
}

bool DataTypeList::ScalarInfo::operator<(const ScalarInfo& other) const {
    if (isFloatingPoint != other.isFloatingPoint)
        return isFloatingPoint;
    if (isSigned != other.isSigned)
        return isSigned;
    return size < other.size;
}

DataType* DataTypeList::getByName(const std::string& name) const {
    auto it = m_dataTypes.find(name);
    if (it != m_dataTypes.end()) {
        return it->second;
    }
    return nullptr;
}

ScalarDataType* DataTypeList::getScalar(bool isFloatingPoint, bool isSigned, size_t size) {
    auto it = m_scalarDataTypes.find({isFloatingPoint, isSigned, size});
    if (it != m_scalarDataTypes.end()) {
        return it->second;
    }
    return nullptr;
}

void DataTypeList::onObjectAdded(DataType* dataType) {
    m_dataTypes[dataType->getName()] = dataType;
    if (auto scalarDataType = dynamic_cast<ScalarDataType*>(dataType)) {
        ScalarInfo scalarInfo = {
            scalarDataType->isFloatingPoint(),
            scalarDataType->isSigned(),
            scalarDataType->getSize()
        };
        m_scalarDataTypes[scalarInfo] = scalarDataType;
    }
}

void DataTypeList::onObjectRemoved(DataType* dataType) {
    m_dataTypes.erase(dataType->getName());
    if (auto scalarDataType = dynamic_cast<ScalarDataType*>(dataType)) {
        ScalarInfo scalarInfo = {
            scalarDataType->isFloatingPoint(),
            scalarDataType->isSigned(),
            scalarDataType->getSize()
        };
        m_scalarDataTypes.erase(scalarInfo);
    }
}