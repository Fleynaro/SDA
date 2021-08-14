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

	enum class Debugger
	{
		DebuggerEngine // WinDbg
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
		bool m_isLoaded;
		bool m_isBase;
	};

	struct DebugRegister
	{
		int m_id;
		int m_index;
		uint64_t m_value;
	};
	
	class IDebugSession
	{
	public:
		virtual Debugger getDebugger() = 0;
		
		virtual void attachProcess(const DebugProcess& process) = 0;

		virtual DebugProcess getProcess() = 0;

		virtual void stepOver() = 0;

		virtual void stepInto() = 0;

		virtual void pause(bool wait = false) = 0;

		virtual void resume() = 0;

		virtual void stop() = 0;

		virtual bool isSuspended() = 0;

		virtual bool isWorking() = 0;
		
		virtual void addBreakpoint(std::uintptr_t address) = 0;

		virtual void removeBreakpoint(std::uintptr_t address) = 0;

		virtual std::list<DebugModule> getModules() = 0;

		virtual void readMemory(std::uintptr_t address, std::vector<uint8_t>& data) = 0;

		virtual std::uintptr_t getInstructionAddress() = 0;

		virtual std::list<DebugRegister> getRegisters() = 0;
	};
};