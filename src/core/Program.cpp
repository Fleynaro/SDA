#include "Program.h"
#include "ProjectManager.h"
#ifdef WIN32
#include "Windows.h"
#endif

CE::Program::Program()
{
	m_projectManager = new ProjectManager(this);
}

fs::path CE::Program::getExecutableDirectory() {
#ifdef WIN32
	char filename[MAX_PATH];
	GetModuleFileName(NULL, filename, MAX_PATH);
	return fs::path(filename).parent_path();
#else
	return "";
#endif
}

CE::ProjectManager* CE::Program::getProjectManager() const
{
	return m_projectManager;
}
