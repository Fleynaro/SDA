#include "Description.h"

Description::Description(const std::string& name, const std::string& comment)
	: m_name(name), m_comment(comment)
{}

const char* Description::getName() {
	return m_name.c_str();
}

void Description::setName(const std::string& name) {
	m_name = name;
}

const char* Description::getComment() {
	return m_comment.c_str();
}

void Description::setComment(const std::string& comment) {
	m_comment = comment;
}
