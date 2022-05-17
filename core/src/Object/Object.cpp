#include "Core/Object/Object.h"
#include "Core/Context.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace sda;

Object::Object(ObjectId* id, const std::string& name)
    : m_id(id ? *id : boost::uuids::random_generator()()), m_name(name)
{}

ObjectId Object::getId() const {
    return m_id;
}

const std::string& Object::getName() const {
    return m_name;
}

void Object::setName(const std::string& name) {
    m_name = name;
}

const std::string& Object::getComment() const {
    return m_comment;
}

void Object::setComment(const std::string& comment) {
    m_comment = comment;
}

bool Object::isTemporary() const {
    return m_temporary;
}

void Object::setTemporary(bool temporary) {
    m_temporary = temporary;
}

boost::json::string Object::serializeId() const {
    return boost::uuids::to_string(m_id);
}

void Object::serialize(boost::json::object& data) const {
    data["uuid"] = serializeId();
    data["name"] = m_name;
    data["comment"] = m_comment;
    data["temporary"] = m_temporary;
}

void Object::deserialize(boost::json::object& data) {
    m_name = data["name"].get_string();
    m_comment = data["comment"].get_string();
}

ContextObject::ContextObject(Context* context, ObjectId* id, const std::string& name)
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