#pragma once
#include <database/Transaction.h>
#include <ghidra_sync/GhidraSync.h>
#include <filesystem>
namespace fs = std::filesystem;

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

		fs::path getProjectsFile();

		Project* loadProject(const fs::path& dir);

		Project* createProject(const fs::path& dir);

		const auto& getProjectEntries();

		void load();

		void save();
	};
};