#pragma once
#include <exception>
#include <filesystem>
#include <string>
#include <list>

namespace CE
{
	class DebugException : public std::exception {
	public: DebugException(const std::string& message) : std::exception(message.c_str()) {}
	};

	struct DebugProcess
	{
		uint64_t m_id;
		std::string m_name;
	};
	
	struct DebugModule
	{
		std::filesystem::path m_path;
		std::uintptr_t m_baseAddress;
		int m_size;
	};
	
	class IDebugSession
	{
	public:
		virtual void attachProcess(const DebugProcess& process) = 0;

		virtual void stepOver() = 0;

		virtual void stepInto() = 0;

		virtual void pause() = 0;

		virtual void resume() = 0;

		virtual void stop() = 0;

		virtual bool isSuspended() = 0;

		virtual bool isWorking() = 0;
		
		virtual void addBreakpoint(std::uintptr_t address) = 0;

		virtual void removeBreakpoint(std::uintptr_t address) = 0;

		virtual const std::list<DebugModule>& getModules() = 0;
	};
};