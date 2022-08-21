#include "Core/Object/ContextObject.h"
#include "Core/Context.h"

using namespace sda;

void ContextObject::notifyModified(Object::ModState state) {
    if (state == Object::ModState::Before) {
        m_context->getCallbacks()->onObjectModified(this);
    }
}

ContextObject::ContextObject(Context* context, Object::Id* id, const std::string& name)
    : Object(id), m_name(name), m_context(context)
{
    if (m_name.empty())
        m_name = std::string("obj_") + std::string(serializeId());
}

const std::string& ContextObject::getName() const {
    return m_name;
}

void ContextObject::setName(const std::string& name) {
    notifyModified(Object::ModState::Before);
    m_name = name;
    notifyModified(Object::ModState::After);
}

const std::string& ContextObject::getComment() const {
    return m_comment;
}

void ContextObject::setComment(const std::string& comment) {
    notifyModified(Object::ModState::Before);
    m_comment = comment;
    notifyModified(Object::ModState::After);
}

void ContextObject::serialize(boost::json::object& data) const {
    Object::serialize(data);
    data["name"] = m_name;
    data["comment"] = m_comment;
}

void ContextObject::deserialize(boost::json::object& data) {
    notifyModified(Object::ModState::Before);
    m_name = data["name"].get_string().c_str();
    m_comment = data["comment"].get_string().c_str();
}