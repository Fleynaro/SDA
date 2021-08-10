#pragma once
#include "DebugSession.h"

namespace CE
{
	std::list<Debugger> GetAvailableDebuggers();

	std::string GetDubuggerName(Debugger debugger);

	IDebugSession* CreateDebugSession(Debugger debugger);

	std::list<DebugProcess> GetProcesses();
};