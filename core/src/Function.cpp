#include "Core/Function.h"
#include "Core/Context.h"

using namespace sda;

Function::Function(Context* context, ObjectId* id, const std::string& name, int64_t offset)
    : ContextObject(context, id, name), m_offset(offset)
{
    m_context->getFunctions()->add(std::unique_ptr<Function>(this));
}

int64_t Function::getOffset() const {
    return m_offset;
}

void Function::setOffset(int64_t offset) {
    m_context->getCallbacks()->onObjectModified(this);
    m_offset = offset;
}

Function* Function::clone() const {
    auto clonedFunc = new Function(m_context);
    boost::json::object data;
    serialize(data);
    clonedFunc->deserialize(data);
    return clonedFunc;
}

void Function::serialize(boost::json::object& data) const {
    Object::serialize(data);
    data["collection"] = "functions";
    data["offset"] = m_offset;
}

void Function::deserialize(boost::json::object& data) {
    m_context->getCallbacks()->onObjectModified(this);
    Object::deserialize(data);
    m_offset = data["offset"].get_int64();
}

void Function::destroy() {
    m_context->getFunctions()->remove(getId());
}