#ifdef WIN32
#include "DebuggerEngine.h"

HRESULT CE::DebuggerEngineEventCallbacks::LoadModule(ULONG64 imghdl, ULONG64 baseoff, ULONG modsize, PCSTR modname,
                                                     PCSTR imgname, ULONG chksum, ULONG timestamp) {
	printf(
		"LoadModule imghdl[0x%llx] baseoff[0x%llx] modsize [0x%lx] modname[%s] imgname[%s] chksum [0x%lx] timestamp [0x%lx]",
		imghdl, baseoff, modsize, modname, imgname, chksum, timestamp);

	DebugModule debugModule;
	debugModule.m_path = imgname;
	debugModule.m_baseAddress = baseoff;
	debugModule.m_size = modsize;
	m_debugSession->m_modules.push_back(debugModule);
	return DEBUG_STATUS_NO_CHANGE;
}

#endif