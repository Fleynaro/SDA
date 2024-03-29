#include "ProjectManagerPanel.h"
#include "panels/ProjectPanel.h"
#include "imgui_wrapper/controls/Text.h"

void GUI::ProjectManagerPanel::ProjectCreatorPanel::renderPanel() {
	Text::Text("Choose a directory for your project.").show();
	m_fileDialog.show();
	NewLine();
	if (Button::StdButton("Create").present()) {
		m_prjManagerPanel->createNewProject(m_fileDialog.getPath());
		m_window->close();
	}
}

GUI::ProjectManagerPanel::ProjectManagerPanel(CE::Program* program)
	: AbstractPanel("Project manager"), m_controller(program)
{
	auto listView = new TableListView(&m_controller.m_listModel, {
		                                  ColInfo("Project path")
	                                  });
	m_tableListView = new TableListViewSelector(listView, new Button::StdButton("open"));
	m_tableListView->handler([&](CE::ProjectManager::ProjectEntry* projectEntry)
	{
		openProject(projectEntry);
	});
}

GUI::ProjectManagerPanel::~ProjectManagerPanel() {
	delete m_tableListView;
}

GUI::StdWindow* GUI::ProjectManagerPanel::createStdWindow() {
	const auto window = new StdWindow(this);
	window->setFlags(
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar);
	window->setFullscreen(true);
	return window;
}

void GUI::ProjectManagerPanel::renderPanel() {
	if (m_tryToLoad) {
		m_controller.load();
		m_tryToLoad = false;
	}
	renderProjectList();
	Show(m_prjCreatorWin);
	Show(m_projectWin);

	if (m_isImguiDemo)
		ImGui::ShowDemoWindow(&m_isImguiDemo);
}

void GUI::ProjectManagerPanel::renderProjectList() {
	Text::Text("You can create a new project or choose the existing one.").show();
	
	if (Button::StdButton("Create a new project").present()) {
		openProjectCreatorPanel();
	}
	SameLine();
	if (Button::StdButton("Open demo project").present()) {
		openTestProjectPanel();
	}
	NewLine();

	if (m_controller.hasProjects()) {
		m_tableListView->show();
	}
	else {
		Text::Text("No projects.").show();
	}

	NewLine();
	NewLine();
	if (Button::StdButton("Open imgui demo").present()) {
		m_isImguiDemo = true;
	}
}

void GUI::ProjectManagerPanel::createNewProject(const fs::path& dir) {
	const auto project = m_controller.createNewProject(dir);
	loadProject(project);
}

void GUI::ProjectManagerPanel::openProject(CE::ProjectManager::ProjectEntry* projectEntry) {
	const auto project = m_controller.openProject(projectEntry);
	loadProject(project);
}

void GUI::ProjectManagerPanel::loadProject(CE::Project* project) {
	m_controller.loadProject(project);
	delete m_projectWin;
	m_projectWin = (new ProjectPanel(project))->createStdWindow();
}

void GUI::ProjectManagerPanel::openTestProjectPanel() {
	if(!m_controller.m_testProject)
		m_controller.createDemoProject();
	delete m_projectWin;
	m_projectWin = (new ProjectPanel(m_controller.m_testProject))->createStdWindow();
}

void GUI::ProjectManagerPanel::openProjectCreatorPanel() {
	delete m_prjCreatorWin;
	m_prjCreatorWin = (new ProjectCreatorPanel(this))->createStdWindow();
}
