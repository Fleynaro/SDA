#pragma once
#include "main.h"

namespace CE::Decompiler
{
	class WarningContainer
	{
		std::list<std::string> m_notices;
		std::list<std::string> m_warnings;
		std::list<std::string> m_errors;
	public:
		WarningContainer()
		{}

		auto& getNotices() {
			return m_notices;
		}

		auto& getWarnings() {
			return m_warnings;
		}

		auto& getErrors() {
			return m_errors;
		}

		void addNotice(const std::string& message) {
			m_notices.push_back(message);
		}

		void addWarning(const std::string& message) {
			m_warnings.push_back(message);
		}

		void addError(const std::string& message) {
			m_errors.push_back(message);
		}

		bool hasAnything() {
			return !m_notices.empty() || !m_warnings.empty() || !m_errors.empty();
		}

		std::string getAllMessages() {
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
				result += "- " +  error + "\n";
			}

			return result;
		}
	};

	class IWarningGenerator {
	public:
		virtual WarningContainer* getWarningContainer() = 0;
	};
};