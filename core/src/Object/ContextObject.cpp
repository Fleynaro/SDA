#include "Core/Object/ContextObject.h"
#include "Core/Context.h"

using namespace sda;

ContextObject::ContextObject(Context* context, Object::Id* id, const std::string& name)
    : Object(id, name), m_context(context)
{}

void ContextObject::setName(const std::string& name) {
    m_context->getCallbacks()->onObjectModified(this);
    Object::setName(name);
}

void ContextObject::setComment(const std::string& comment) {
    m_context->getCallbacks()->onObjectModified(this);
    Object::setComment(comment);
}

void ContextObject::deserialize(boost::json::object& data) {
    m_context->getCallbacks()->onObjectModified(this);
    Object::deserialize(data);
}