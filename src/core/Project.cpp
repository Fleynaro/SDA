#include "Project.h"
#include "ProjectManager.h"
#include <Program.h>
#include <managers/Managers.h>
#include <cmrc/cmrc.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fstream>
#include <iostream>

CMRC_DECLARE(resources);
using namespace CE;

Project::Project(ProjectManager* projectManager, const fs::path& dir)
	: m_projectManager(projectManager), m_directory(dir)
{}

Project::~Project() {
	if (m_allManagersHaveBeenLoaded) {
		delete m_addrSpaceManager;
		delete m_imageManager;
		delete m_functionManager;
		delete m_symbolManager;
		delete m_symbolTableManager;
		delete m_typeManager;
	}
	if (m_transaction != nullptr)
		delete m_transaction;
	if (m_db != nullptr)
		delete m_db;
}

ProjectManager* Project::getProjectManager() const
{
	return m_projectManager;
}

Program* Project::getProgram() const
{
	return m_projectManager->getProgram();
}

void Project::initTransaction() {
	m_transaction = new DB::Transaction(m_db);
}

void Project::load() const
{
	getTypeManager()->loadBefore();
	getSymbolManager()->loadSymbols();
	getSymTableManager()->loadSymTables();
	getAddrSpaceManager()->loadAddressSpaces();
	getImageManager()->loadImages();
	getFunctionManager()->loadFunctions();
	getTypeManager()->loadAfter();
}

void Project::save() const
{
	// save data into database
	m_transaction->commit();
}

void Project::initManagers()
{
	m_typeManager = new TypeManager(this);
	m_functionManager = new FunctionManager(this);
	m_symbolManager = new SymbolManager(this);
	m_symbolTableManager = new SymbolTableManager(this);
	m_addrSpaceManager = new AddressSpaceManager(this);
	m_imageManager = new ImageManager(this);
	m_allManagersHaveBeenLoaded = true;
}

void Project::createTablesInDatabase() const
{
	using namespace SQLite;

	// open the embedded file system and find the required resource
	const auto fs = cmrc::resources::get_filesystem();
	const auto create_general_db_res = fs.open("create_general_db.sql");
	const auto sql_query = std::string(create_general_db_res.begin(), create_general_db_res.end());

	try {
		m_db->exec(sql_query);
	}
	catch (Exception e) {
		std::cout << "!!! createTablesInDatabase error: " << std::string(e.what());
	}
}

void Project::initDataBase(const fs::path& file)
{
	const auto filedb = m_directory / file;
	const bool filedbExisting = exists(filedb);

	// init database
	m_db = new Database(filedb.string(), OPEN_READWRITE | OPEN_CREATE);
	
	// if data base didn't exist then create tables
	if (!filedbExisting) {
		createTablesInDatabase();
	}

	initTransaction();
}

Database& Project::getDB() const
{
	return *m_db;
}

TypeManager* Project::getTypeManager() const
{
	return m_typeManager;
}

SymbolManager* Project::getSymbolManager() const
{
	return m_symbolManager;
}

SymbolTableManager* Project::getSymTableManager() const
{
	return m_symbolTableManager;
}

FunctionManager* Project::getFunctionManager() const
{
	return m_functionManager;
}

AddressSpaceManager* Project::getAddrSpaceManager() const
{
	return m_addrSpaceManager;
}

ImageManager* Project::getImageManager() const
{
	return m_imageManager;
}

DB::ITransaction* Project::getTransaction() const
{
	return m_transaction;
}

const fs::path& Project::getDirectory() const
{
	return m_directory;
}

fs::path Project::getImagesDirectory() const
{
	return m_directory / fs::path("images");
}