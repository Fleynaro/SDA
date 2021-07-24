#pragma once
#include "gui/controllers/ProjectManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Button.h"
#include "imgui_wrapper/controls/Input.h"
#include "imgui_wrapper/widgets/file_dialog/FileDialog.h"


namespace GUI
{
	class ProjectManagerPanel : public AbstractPanel
	{
		class ProjectCreatorPanel : public AbstractPanel
		{
			ProjectManagerPanel* m_prjManagerPanel;
			Widget::FileDialog m_fileDialog;
		public:
			ProjectCreatorPanel(ProjectManagerPanel* prjManagerPanel)
				: AbstractPanel("Create a new project"), m_prjManagerPanel(prjManagerPanel)
			{
				m_fileDialog = Widget::FileDialog("Choose Directory");
				m_fileDialog.setPath(m_prjManagerPanel->m_controller.findDefaultPath());
			}

			StdWindow* createStdWindow() {
				const auto window = new StdWindow(this, ImGuiWindowFlags_NoResize);
				window->getSize() = ImVec2(500, 120);
				return window;
			}

		private:
			void renderPanel() override;
		};
		
		ProjectManagerController m_controller;
		TableListViewSelector<CE::ProjectManager::ProjectEntry*>* m_tableListView;
		StdWindow* m_demoWin = nullptr;
		StdWindow* m_prjCreatorWin = nullptr;
		PopupModalWindow* m_warningModalWin = nullptr;
		std::list<StdWindow*> m_projectWins;
		bool m_tryToLoad = true;
	public:
		ProjectManagerPanel(CE::Program* program);

		~ProjectManagerPanel() override;

		StdWindow* createStdWindow();

	private:
		void renderPanel() override;

		void renderProjectList();

		void renderProjectWindows();

		void createWarningWindow(const WarningException& ex);

		void createNewProject(const fs::path& dir);

		void openProject(CE::ProjectManager::ProjectEntry* projectEntry);

		void loadProject(CE::Project* project);

		void openProjectCreatorPanel();
	};
};
