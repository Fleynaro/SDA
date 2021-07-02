#include "Description.h"

Description::Description(const std::string& name, const std::string& comment)
	: m_name(name), m_comment(comment)
{}

const std::string Description::getName() {
	return m_name;
}

void Description::setName(const std::string& name) {
	m_name = name;
}

const std::string Description::getComment() {
	return m_comment;
}

void Description::setComment(const std::string& comment) {
	m_comment = comment;
}
