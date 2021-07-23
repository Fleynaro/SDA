#pragma once
#include "ImageManagerPanel.h"
#include "Program.h"
#include "Project.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "nlohmann/json.hpp"

namespace GUI
{
	class WindowSaveProvider
	{
		fs::path m_file;
		json m_result;
	public:
		WindowSaveProvider(const fs::path& file)
			: m_file(file)
		{}

		void load() {
			std::ifstream file(m_file);
			std::string content;
			file >> content;
			m_result = json::parse(content);
		}

		void save() {
			std::ofstream file(m_file);
			const auto content = m_result.dump();
			file << content;
		}

		bool hasWindow(const std::string& name) const {
			return m_result.find(name) != m_result.end();
		}

		void loadWindow(StdWindow* window, const std::string& name) {
			const auto it = m_result.find(name);
			if (it == m_result.end())
				return;
			const auto info = *it;
			{
				const auto pos = info["pos"];
				//window->getPos() = ImVec2(pos["x"].get<float>(), pos["y"].get<float>());
			}
			{
				const auto size = info["size"];
				//window->getSize() = ImVec2(size["x"].get<float>(), size["y"].get<float>());
			}
			//window->m_applyPosAndSize = true;
		}

		void saveWindow(StdWindow* window, const std::string& name) {
			json info;
			{
				json pos;
				pos["x"] = window->getPos().x;
				pos["y"] = window->getPos().y;
				info["pos"] = pos;
			}
			{
				json size;
				size["x"] = window->getSize().x;
				size["y"] = window->getSize().y;
				info["size"] = size;
			}
			m_result[name] = info;
		}
	};
	
	class ProjectPanel : public AbstractPanel
	{
		CE::Project* m_project;

		class StdWorkspace : public AbstractPanel
		{
			friend class ProjectPanel;
			ProjectPanel* m_projectPanel;
			StdWindow* m_imageViewerWindow = nullptr;
			StdWindow* m_imageContentViewerWindow = nullptr;
		public:
			StdWorkspace(ProjectPanel* projectPanel)
				: m_projectPanel(projectPanel)
			{
				if (exists(getGuiFile())) {
					loadWindows();
				}
				else {
					firstInitWindows();
				}
			}

			~StdWorkspace() {
				saveWindows();
				delete m_imageViewerWindow;
			}
		
		private:
			void renderPanel() override {
				m_imageViewerWindow->setDockSpace(m_projectPanel->m_dockspaceId);
				Show(m_imageViewerWindow);
			}

			void renderMenuBar() override {
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save")) {
						
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View"))
				{
					if (ImGui::MenuItem("Image Viewer", nullptr, m_imageViewerWindow != nullptr)) {
						if (m_imageViewerWindow)
							m_imageViewerWindow->close();
						else createImageViewerWindow();
					}
					ImGui::EndMenu();
				}
			}

			fs::path getGuiFile() {
				return m_projectPanel->m_project->getProgram()->getExecutableDirectory() / "gui.json";
			}

			void loadWindows() {
				WindowSaveProvider saver(getGuiFile());
				saver.load();
				saver.loadWindow(m_projectPanel->m_window, "Project");
				if (saver.hasWindow("ImageViewer")) {
					saver.loadWindow(createImageViewerWindow(), "ImageViewer");
				}
			}

			void saveWindows() {
				WindowSaveProvider saver(getGuiFile());
				saver.saveWindow(m_projectPanel->m_window, "Project");
				if(m_imageViewerWindow)
					saver.saveWindow(m_imageViewerWindow, "ImageViewer");
				saver.save();
			}

			void firstInitWindows() {
				m_projectPanel->m_window->getSize() = ImVec2(1000, 800);
				createImageViewerWindow();
			}

			StdWindow* createImageViewerWindow() {
				return m_imageViewerWindow = new StdWindow(new ImageManagerPanel(m_projectPanel->m_project->getImageManager()));
			}

			
		};

		StdWorkspace* m_workspace = nullptr;
		ImGuiID m_dockspaceId;
	public:
		
		ProjectPanel(CE::Project* project)
			: AbstractPanel("Project: " + project->getDirectory().string() + "###project"), m_project(project)
		{}

		~ProjectPanel() override {
			delete m_workspace;
		}

		StdWindow* createStdWindow() {
			const auto window = new StdWindow(this, ImGuiWindowFlags_MenuBar);
			m_workspace = new StdWorkspace(this);
			return window;
		}
	
	protected:
		void renderPanel() override {
			ImGui::DockSpace(m_dockspaceId = ImGui::GetID("ProjectDockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
			m_workspace->renderPanel();
		}

		void renderMenuBar() override {
			m_workspace->renderMenuBar();
		}
	};
};
