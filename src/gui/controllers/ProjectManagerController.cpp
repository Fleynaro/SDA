#include "ProjectManagerController.h"
#include "dec_test_samples.h"
#include "Program.h"
#include <fstream>

GUI::ProjectManagerController::ProjectManagerController(CE::Program* program)
	: m_program(program), m_listModel(program->getProjectManager())
{}

void GUI::ProjectManagerController::createDemoProject() {
	const auto dir = m_program->getExecutableDirectory() / "demo";
	if (exists(dir))
		remove_all(dir);
	m_testProject = createNewProject(dir);
	loadProject(m_testProject);
	CE::DecTestSamplesPool decTestSamplesPool(m_testProject);
	decTestSamplesPool.fillByTests();
	decTestSamplesPool.analyze();
}

void GUI::ProjectManagerController::load() const {
	const auto projectsFile = m_program->getProjectManager()->getProjectsFile();
	if (!exists(projectsFile)) {
		// create a projects file
		std::ofstream(projectsFile) << "[]";
	}
	m_program->getProjectManager()->load();
}

bool GUI::ProjectManagerController::hasProjects() const {
	return !m_program->getProjectManager()->m_projectEntries.empty();
}

CE::Project* GUI::ProjectManagerController::openProject(CE::ProjectManager::ProjectEntry* projectEntry) const {
	return m_program->getProjectManager()->loadProject(projectEntry->m_dir);
}

CE::Project* GUI::ProjectManagerController::createNewProject(const fs::path& dir) const {
	const auto project = m_program->getProjectManager()->createProject(dir);
	m_program->getProjectManager()->save();
	return project;
}

void GUI::ProjectManagerController::loadProject(CE::Project* project) const {
	project->initDataBase("database.db");
	project->initManagers();
	project->load();
}

fs::path GUI::ProjectManagerController::findDefaultPath() const {
	fs::path path;
	int counter = 1;
	do {
		const auto dirName = "myProject_" + std::to_string(counter);
		path = m_program->getExecutableDirectory() / dirName;
		counter++;
	}
	while (exists(path));
	return path;
}
