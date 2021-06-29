#pragma once
#include <DB/Transaction.h>
#include <GhidraSync/GhidraSync.h>

using namespace SQLite;

namespace CE
{
	class Program;
	class ProjectManager;

	// managers
	class TypeManager;
	class SymbolManager;
	class SymbolTableManager;
	class FunctionManager;
	class AddressSpaceManager;
	class ImageManager;

	namespace Ghidra {
		class Client;
	};

	namespace Symbol {
		class SymbolTable;
	};

	class Project
	{
		ProjectManager* m_projectManager;

		bool m_allManagersHaveBeenLoaded = false;
		DB::ITransaction* m_transaction = nullptr;
		SQLite::Database* m_db = nullptr;

		// the directory is an id for a project
		fs::path m_directory;

		TypeManager* m_typeManager = nullptr;
		SymbolManager* m_symbolManager = nullptr;
		SymbolTableManager* m_symbolTableManager = nullptr;
		FunctionManager* m_functionManager = nullptr;
		AddressSpaceManager* m_addrSpaceManager = nullptr;
		ImageManager* m_imageManager = nullptr;
		Ghidra::Sync* m_ghidraSync;
	public:
		Project(ProjectManager* projectManager, const fs::path& dir)
			: m_projectManager(projectManager), m_directory(dir)
		{
			m_ghidraSync = new Ghidra::Sync(this);
		}

		~Project();

		ProjectManager* getProjectManager();

		Program* getProgram();

		void load();

		void save() {
			// save data into database
			m_transaction->commit();
		}

		void initManagers();

		void initDataBase(const fs::path& file);

		SQLite::Database& getDB();

		TypeManager* getTypeManager();

		SymbolManager* getSymbolManager();

		SymbolTableManager* getSymTableManager();

		FunctionManager* getFunctionManager();

		AddressSpaceManager* getAddrSpaceManager();

		ImageManager* getImageManager();

		DB::ITransaction* getTransaction();

		const fs::path& getDirectory();

		fs::path getImagesDirectory();

		Ghidra::Sync* getGhidraSync();

	private:
		void initTransaction();

		void createTablesInDatabase();
	};

	class ProjectManager
	{
	public:
		struct ProjectEntry
		{
			fs::path m_dir;
		};

	private:
		Program* m_program;
		std::list<ProjectEntry> m_projectEntries;

	public:
		ProjectManager(Program* program)
			: m_program(program)
		{}

		Program* getProgram();

		const fs::path& getProjectsFile();

		Project* loadProject(const fs::path& dir) {
			return new Project(this, dir);
		}

		Project* createProject(const fs::path& dir) {
			ProjectEntry projectEntry;
			projectEntry.m_dir = dir;
			m_projectEntries.push_back(projectEntry);

			auto project = new Project(this, dir);
			if (!fs::exists(project->getDirectory()))
				fs::create_directory(project->getDirectory());
			if (!fs::exists(project->getImagesDirectory()))
				fs::create_directory(project->getImagesDirectory());
			return project;
		}

		const auto& getProjectEntries() {
			return m_projectEntries;
		}

		void load();

		void save();
	};
};