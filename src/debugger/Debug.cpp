#include "Debug.h"

#ifdef WIN32
#include "DebuggerEngine.h"
#include <psapi.h>
#include <tlhelp32.h>
#endif

std::list<CE::Debugger> CE::GetAvailableDebuggers() {
	std::list<Debugger> debuggers;
#ifdef WIN32
	debuggers.push_back(Debugger::DebuggerEngine);
#endif
	return debuggers;
}

std::string CE::GetDubuggerName(Debugger debugger) {
	if (debugger == Debugger::DebuggerEngine)
		return "Debugger Engine (WinDbg)";
	return "";
}

CE::IDebugSession* CE::CreateDebugSession(Debugger debugger) {
	if (debugger == Debugger::DebuggerEngine)
		return new DebuggerEngineSession;
	return nullptr;
}

std::list<CE::DebugProcess> CE::GetProcesses() {
	std::list<DebugProcess> debugProcesses;

#ifdef WIN32
	const auto handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE, 0);
	if (handle) {
		PROCESSENTRY32 process;
		Process32First(handle, &process);
		do {
			DebugProcess debugProcess;
			debugProcess.m_id = process.th32ProcessID;
			debugProcess.m_name = process.szExeFile;
			debugProcesses.push_back(debugProcess);
		} while (Process32Next(handle, &process));
		CloseHandle(handle);
	}
#endif
	
	return debugProcesses;
}