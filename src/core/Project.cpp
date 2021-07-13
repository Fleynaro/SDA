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

Project::Project(ProjectManager* projectManager, const fs::path& dir)
	: m_projectManager(projectManager), m_directory(dir)
{
	m_ghidraSync = new Ghidra::Sync(this);
}

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

Ghidra::Sync* Project::getGhidraSync() const
{
	return m_ghidraSync;
}

Program* ProjectManager::getProgram() const
{
	return m_program;
}

fs::path ProjectManager::getProjectsFile() const
{
	return m_program->getExecutableDirectory() / fs::path("projects.json");
}

Project* ProjectManager::loadProject(const fs::path& dir) {
	return new Project(this, dir);
}

Project* ProjectManager::createProject(const fs::path& dir) {
	ProjectEntry projectEntry;
	projectEntry.m_dir = dir;
	m_projectEntries.push_back(projectEntry);

	const auto project = new Project(this, dir);
	if (!exists(project->getDirectory()))
		create_directory(project->getDirectory());
	if (!exists(project->getImagesDirectory()))
		create_directory(project->getImagesDirectory());
	return project;
}

const auto& ProjectManager::getProjectEntries() const
{
	return m_projectEntries;
}

void ProjectManager::load() {
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

void ProjectManager::save() {
	json json_project_entries;
	for (auto& prjEntry : m_projectEntries) {
		json json_project_entry;
		json_project_entry["path"] = prjEntry.m_dir.string();
		json_project_entries.push_back(json_project_entry);
	}
	std::ofstream file(getProjectsFile());
	if (!file.is_open())
		throw std::logic_error("");
	const auto content = json_project_entries.dump();
	file << content;
}
