#include "Enum.h"

using namespace CE;
using namespace DataType;

int Enum::getSize() {
	return m_size;
}

void Enum::setSize(int size) {
	m_size = size;
}

Enum::Group Enum::getGroup() {
	return Group::Enum;
}

std::string Enum::getViewValue(uint64_t value) {
	const auto it = m_fields.find(value);
	if (it == m_fields.end())
		return UserDefinedType::getViewValue(value);
	return it->second + " (" + UserDefinedType::getViewValue(value) + ")";
}

Enum::FieldMapType& Enum::getFields() {
	return m_fields;
}

bool Enum::removeField(int value) {
	const auto it = m_fields.find(value);
	if (it != m_fields.end()) {
		m_fields.erase(it);
		return true;
	}
	return false;
}

void Enum::addField(std::string name, int value) {
	m_fields[value] = name;
}

void Enum::deleteAll() {
	m_fields.clear();
}
