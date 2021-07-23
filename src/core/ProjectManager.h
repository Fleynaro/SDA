#pragma once
#include <filesystem>
namespace fs = std::filesystem;

namespace CE {
	class Program;
	class Project;
};

namespace CE
{
	class ProjectManager
	{
	public:
		struct ProjectEntry
		{
			fs::path m_dir;
		};

	private:
		Program* m_program;

	public:
		std::list<ProjectEntry> m_projectEntries;
		
		ProjectManager(Program* program)
			: m_program(program)
		{}

		Program* getProgram() const;

		fs::path getProjectsFile() const;

		Project* loadProject(const fs::path& dir);

		Project* createProject(const fs::path& dir);

		void load();

		void save();
	};
};