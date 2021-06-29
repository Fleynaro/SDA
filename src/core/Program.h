#pragma once
#include <main.h>

namespace CE
{
	class ProjectManager;

	class Program
	{
		ProjectManager* m_projectManager;
	public:
		Program();

		fs::path getExecutableDirectory() {
			char filename[MAX_PATH];
			GetModuleFileName(NULL, filename, MAX_PATH);
			return fs::path(filename).parent_path();
		}

		ProjectManager* getProjectManager() {
			return m_projectManager;
		}
	};
};