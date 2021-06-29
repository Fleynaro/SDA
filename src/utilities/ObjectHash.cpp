#include "ObjectHash.h"

void ObjectHash::addValue(std::string value) {
	m_hashContent += "{" + value + "}";
}

void ObjectHash::addValue(int value) {
	addValue((int64_t)value);
}

void ObjectHash::addValue(int64_t value) {
	addValue(std::to_string(value));
}

ObjectHash::Hash ObjectHash::getHash() {
	return m_hash * 31 + hash(m_hashContent);
}

void ObjectHash::join(ObjectHash& hash) {
	m_hash = m_hash * 31 + hash.getHash();
}

void ObjectHash::add(ObjectHash& hash) {
	m_hash = m_hash + hash.getHash();
}

ObjectHash::Hash ObjectHash::hash(std::string string) {
	Hash h = 1125899906842597L;
	for (int i = 0; i < string.length(); i++) {
		h = 31 * h + string.at(i);
	}
	return h;
}
