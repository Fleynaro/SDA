#include "Core/Object.h"

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

std::list<Object::KeyValue> Object::getKeyValues() const {
    return {
        {"id", KeyValue::Text, boost::uuids::to_string(m_id)}
    };
}