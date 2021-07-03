#include "Program.h"
#include <Project.h>

CE::Program::Program()
{
	m_projectManager = new ProjectManager(this);
}

fs::path CE::Program::getExecutableDirectory() {
	char filename[MAX_PATH];
	GetModuleFileName(NULL, filename, MAX_PATH);
	return fs::path(filename).parent_path();
}

CE::ProjectManager* CE::Program::getProjectManager() const
{
	return m_projectManager;
}
