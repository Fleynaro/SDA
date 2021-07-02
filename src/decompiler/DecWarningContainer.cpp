#include "DecWarningContainer.h"

auto& CE::Decompiler::WarningContainer::getNotices() {
	return m_notices;
}

auto& CE::Decompiler::WarningContainer::getWarnings() {
	return m_warnings;
}

auto& CE::Decompiler::WarningContainer::getErrors() {
	return m_errors;
}

void CE::Decompiler::WarningContainer::addNotice(const std::string& message) {
	m_notices.push_back(message);
}

void CE::Decompiler::WarningContainer::addWarning(const std::string& message) {
	m_warnings.push_back(message);
}

void CE::Decompiler::WarningContainer::addError(const std::string& message) {
	m_errors.push_back(message);
}

bool CE::Decompiler::WarningContainer::hasAnything() {
	return !m_notices.empty() || !m_warnings.empty() || !m_errors.empty();
}

std::string CE::Decompiler::WarningContainer::getAllMessages() {
	std::string result;

	result += "Notices:\n";
	for (auto notice : m_notices) {
		result += "- " + notice + "\n";
	}

	result += "Warnings:\n";
	for (auto warning : m_warnings) {
		result += "- " + warning + "\n";
	}

	result += "Errors:\n";
	for (auto error : m_errors) {
		result += "- " + error + "\n";
	}

	return result;
}
