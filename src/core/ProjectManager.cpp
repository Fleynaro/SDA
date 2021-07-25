#include "ProjectManager.h"
#include "Exception.h"
#include "Program.h"
#include "Project.h"
#include <fstream>

using namespace CE;

Program* ProjectManager::getProgram() const
{
	return m_program;
}

fs::path ProjectManager::getProjectsFile() const
{
	return m_program->getExecutableDirectory() / fs::path("projects.json");
}

Project* ProjectManager::loadProject(const fs::path& dir) {
	if (!exists(dir))
		throw NotFoundException("cannot load the project");
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

void ProjectManager::load() {
	std::ifstream file(getProjectsFile());
	if (!file.is_open())
		throw NotFoundException("cannot open file: " + getProjectsFile().string());
	std::string content;
	file >> content;
	m_projectEntries.clear();
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
		throw NotFoundException("cannot open file: " + getProjectsFile().string());
	const auto content = json_project_entries.dump();
	file << content;
}
