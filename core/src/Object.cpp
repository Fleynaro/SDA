#include "Core/Object.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

using namespace sda;

Object::Object()
    : m_id(boost::uuids::random_generator()())
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

void Object::serialize(boost::json::object& data) const {
    data["uuid"] = boost::uuids::to_string(m_id);
    data["name"] = m_name;
    data["comment"] = m_comment;
}

void Object::deserialize(boost::json::object& data) {
    std::string uuid(data["uuid"].get_string());
    m_id = boost::uuids::string_generator()(uuid);
    m_name = data["name"].get_string();
    m_comment = data["comment"].get_string();
}