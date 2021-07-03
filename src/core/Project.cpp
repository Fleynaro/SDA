#include "Project.h"
#include <Program.h>
#include <ghidra_sync/GhidraSync.h>
#include <managers/Managers.h>
#include <cmrc/cmrc.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fstream>
#include <sstream>
#include <iostream>

CMRC_DECLARE(resources);
using namespace CE;

Project::~Project() {
	if (m_allManagersHaveBeenLoaded) {
		delete m_addrSpaceManager;
		delete m_imageManager;
		delete m_functionManager;
		delete m_symbolManager;
		delete m_symbolTableManager;
		delete m_typeManager;
	}
	if (m_ghidraSync != nullptr) {
		delete m_ghidraSync;
	}
	if (m_transaction != nullptr)
		delete m_transaction;
	if (m_db != nullptr)
		delete m_db;
}

ProjectManager* CE::Project::getProjectManager() {
	return m_projectManager;
}

Program* CE::Project::getProgram() {
	return m_projectManager->getProgram();
}

void Project::initTransaction() {
	m_transaction = new DB::Transaction(m_db);
}

void Project::load()
{
	getTypeManager()->loadBefore();
	getSymbolManager()->loadSymbols();
	getSymTableManager()->loadSymTables();
	getAddrSpaceManager()->loadAddressSpaces();
	getImageManager()->loadImages();
	getFunctionManager()->loadFunctions();
	getTypeManager()->loadAfter();
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

void CE::Project::createTablesInDatabase()
{
	using namespace SQLite;

	// open the embedded file system and find the required resource
	auto fs = cmrc::resources::get_filesystem();
	auto create_general_db_res = fs.open("create_general_db.sql");
	auto sql_query = std::string(create_general_db_res.begin(), create_general_db_res.end());

	try {
		m_db->exec(sql_query);
	}
	catch (SQLite::Exception e) {
		std::cout << "!!! createTablesInDatabase error: " << std::string(e.what());
	}
}

void CE::Project::initDataBase(const fs::path& file)
{
	auto filedb = m_directory / file;
	bool filedbExisting = fs::exists(filedb);

	// init database
	m_db = new SQLite::Database(filedb.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
	
	// if data base didn't exist then create tables
	if (!filedbExisting) {
		createTablesInDatabase();
	}

	initTransaction();
}

SQLite::Database& Project::getDB() {
	return *m_db;
}

TypeManager* Project::getTypeManager() {
	return m_typeManager;
}

SymbolManager* Project::getSymbolManager() {
	return m_symbolManager;
}

SymbolTableManager* Project::getSymTableManager() {
	return m_symbolTableManager;
}

FunctionManager* Project::getFunctionManager() {
	return m_functionManager;
}

AddressSpaceManager* CE::Project::getAddrSpaceManager() {
	return m_addrSpaceManager;
}

ImageManager* CE::Project::getImageManager() {
	return m_imageManager;
}

DB::ITransaction* Project::getTransaction() {
	return m_transaction;
}

const fs::path& CE::Project::getDirectory() {
	return m_directory;
}

fs::path CE::Project::getImagesDirectory() {
	return m_directory / fs::path("images");
}

Ghidra::Sync* Project::getGhidraSync() {
	return m_ghidraSync;
}

Program* CE::ProjectManager::getProgram() {
	return m_program;
}

fs::path CE::ProjectManager::getProjectsFile() {
	return m_program->getExecutableDirectory() / fs::path("projects.json");
}

Project* CE::ProjectManager::loadProject(const fs::path& dir) {
	return new Project(this, dir);
}

Project* CE::ProjectManager::createProject(const fs::path& dir) {
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

const auto& CE::ProjectManager::getProjectEntries() {
	return m_projectEntries;
}

void CE::ProjectManager::load() {
	std::ifstream file(getProjectsFile());
	if (!file.is_open())
		throw std::logic_error("");
	std::string content;
	file >> content;
	auto json_project_entries = json::parse(content);
	for (const auto& json_project_entry : json_project_entries) {
		ProjectEntry projectEntry;
		projectEntry.m_dir = json_project_entry["path"].get<std::string>();
		m_projectEntries.push_back(projectEntry);
	}
}

void CE::ProjectManager::save() {
	json json_project_entries;
	for (auto& prjEntry : m_projectEntries) {
		json json_project_entry;
		json_project_entry["path"] = prjEntry.m_dir.string();
		json_project_entries.push_back(json_project_entry);
	}
	std::ofstream file(getProjectsFile());
	if (!file.is_open())
		throw std::logic_error("");
	auto content = json_project_entries.dump();
	file << content;
}
