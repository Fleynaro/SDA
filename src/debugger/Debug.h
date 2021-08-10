#pragma once
#include "DebugSession.h"

namespace CE
{
	enum class Debugger
	{
		DebuggerEngine // WinDbg
	};

	std::list<Debugger> GetAvailableDebuggers();

	std::string GetDubuggerName(Debugger debugger);

	IDebugSession* CreateDebugSession(Debugger debugger);

	std::list<DebugProcess> GetProcesses();
};