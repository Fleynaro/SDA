#include "SDA/Core/Object/Object.h"
#include "SDA/Core/Context.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace sda;

Object::Object(Id* id)
    : m_id(id ? *id : boost::uuids::random_generator()())
{}

Object::Id Object::getId() const {
    return m_id;
}

const std::list<Object*>& Object::getParents() const {
    return m_parents;
}

void Object::addParent(Object* parent) {
    m_parents.push_back(parent);
}

void Object::removeParent(Object* parent) {
    m_parents.remove(parent);
}

boost::json::string Object::serializeId() const {
    return boost::uuids::to_string(getId());
}

void Object::serialize(boost::json::object& data) const {
    data["uuid"] = serializeId();
}