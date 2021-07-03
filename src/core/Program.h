#pragma once
#include <filesystem>
namespace fs = std::filesystem;

namespace CE
{
	class ProjectManager;

	class Program
	{
		ProjectManager* m_projectManager;
	public:
		Program();

		fs::path getExecutableDirectory();

		ProjectManager* getProjectManager() const;
	};
};