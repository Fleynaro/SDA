#pragma once
#include "Project.h"
#include "imgui_wrapper/controls/AbstractPanel.h"

namespace GUI
{
	class ProjectPanel : public AbstractPanel
	{
	public:
		CE::Project* m_project;
		
		ProjectPanel(CE::Project* project)
			: AbstractPanel("Project: " + project->getDirectory().string()), m_project(project)
		{
			
		}

		~ProjectPanel() override {
			
		}

	protected:
		void renderPanel() override {
			
		}
	};
};
