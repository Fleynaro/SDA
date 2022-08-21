#include "Core/Symbol/Symbol.h"
#include "Core/DataType/DataType.h"

using namespace sda;

Symbol::Symbol(
    Context* context,
    Object::Id* id,
    const std::string& name,
    DataType* dataType)
    : ContextObject(context, id, name),
    m_dataType(dataType)
{}

DataType* Symbol::getDataType() const {
    return m_dataType;
}

void Symbol::setDataType(DataType* dataType) {
    notifyModified(Object::ModState::Before);
    m_dataType = dataType;
    notifyModified(Object::ModState::After);
}

void Symbol::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
    data["collection"] = Collection;
    data["data_type"] = m_dataType->serializeId();
}

void Symbol::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);

    m_dataType = m_context->getDataTypes()->get(data["data_type"]);
}

void Symbol::destroy() {
    m_context->getSymbols()->remove(this);
}