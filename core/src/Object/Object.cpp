#include "Core/Object/Object.h"
#include "Core/Context.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace sda;

Object::Object(Id* id)
    : m_id(id ? *id : boost::uuids::random_generator()())
{}

Object::Id Object::getId() const {
    return m_id;
}

void Object::setTemporary(bool temporary) {
    m_temporary = temporary;
}

boost::json::string Object::serializeId() const {
    return boost::uuids::to_string(getId());
}

void Object::serialize(boost::json::object& data) const {
    data["uuid"] = serializeId();
    data["temporary"] = m_temporary;
}