#include "Core/Object/ContextObject.h"
#include "Core/Context.h"

using namespace sda;

ContextObject::ContextObject(Context* context, Object::Id* id, const std::string& name)
    : Object(id), m_name(name), m_context(context)
{}

const std::string& ContextObject::getName() const {
    return m_name;
}

void ContextObject::setName(const std::string& name) {
    m_context->getCallbacks()->onObjectModified(this);
    m_name = name;
}

const std::string& ContextObject::getComment() const {
    return m_comment;
}

void ContextObject::setComment(const std::string& comment) {
    m_context->getCallbacks()->onObjectModified(this);
    m_comment = comment;
}

void ContextObject::serialize(boost::json::object& data) const {
    Object::serialize(data);
    data["name"] = m_name;
    data["comment"] = m_comment;
}

void ContextObject::deserialize(boost::json::object& data) {
    m_context->getCallbacks()->onObjectModified(this);
    m_name = data["name"].get_string();
    m_comment = data["comment"].get_string();
}