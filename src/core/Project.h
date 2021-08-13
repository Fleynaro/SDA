#pragma once
#include <database/Transaction.h>
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


	class Project
	{
		ProjectManager* m_projectManager;

		bool m_allManagersHaveBeenLoaded = false;
		DB::ITransaction* m_transaction = nullptr;
		Database* m_db = nullptr;

		// the directory is an id for a project
		fs::path m_directory;

		TypeManager* m_typeManager = nullptr;
		SymbolManager* m_symbolManager = nullptr;
		SymbolTableManager* m_symbolTableManager = nullptr;
		FunctionManager* m_functionManager = nullptr;
		AddressSpaceManager* m_addrSpaceManager = nullptr;
		ImageManager* m_imageManager = nullptr;
	public:
		Project(ProjectManager* projectManager, const fs::path& dir);

		~Project();

		ProjectManager* getProjectManager() const;

		Program* getProgram() const;

		void load() const;

		void save() const;

		void initManagers();

		void initDataBase(const fs::path& file);

		Database& getDB() const;

		TypeManager* getTypeManager() const;

		SymbolManager* getSymbolManager() const;

		SymbolTableManager* getSymTableManager() const;

		FunctionManager* getFunctionManager() const;

		AddressSpaceManager* getAddrSpaceManager() const;

		ImageManager* getImageManager() const;

		DB::ITransaction* getTransaction() const;

		const fs::path& getDirectory() const;

		fs::path getImagesDirectory() const;

	private:
		void initTransaction();

		void createTablesInDatabase() const;
	};
};