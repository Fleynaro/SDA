#pragma once
#include <list>
#include <string>

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

		auto& getNotices();

		auto& getWarnings();

		auto& getErrors();

		void addNotice(const std::string& message);

		void addWarning(const std::string& message);

		void addError(const std::string& message);

		bool hasAnything() const;

		std::string getAllMessages();
	};

	class IWarningGenerator {
	public:
		virtual WarningContainer* getWarningContainer() = 0;
	};
};