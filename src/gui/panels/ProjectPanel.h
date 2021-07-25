#pragma once
#include "ImageContentViewerPanel.h"
#include "ImageManagerPanel.h"
#include "Program.h"
#include "Project.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "nlohmann/json.hpp"

namespace GUI
{
	class ProjectPanel : public AbstractPanel
	{
		CE::Project* m_project;

		class StdWorkspace : public AbstractPanel
		{
			friend class ProjectPanel;

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

				void addWindow(const std::string& name) {
					m_result[name] = true;
				}
			};
			
			ProjectPanel* m_projectPanel;
			CE::Project* m_project;
			StdWindow* m_imageViewerWindow = nullptr;
			WindowManager m_winManager;
			StdWindow* m_messageWindow = nullptr;
		public:
			StdWorkspace(ProjectPanel* projectPanel)
				: m_projectPanel(projectPanel), m_project(projectPanel->m_project)
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
				m_winManager.m_dockSpaceId = m_projectPanel->m_dockSpaceId;
				m_winManager.show();
				Show(m_imageViewerWindow);
				Show(m_messageWindow);
			}

			void renderMenuBar() override {
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save", nullptr, false, m_project->getTransaction()->hasNewItems())) {
						m_project->getTransaction()->commit();
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
				return m_project->getProgram()->getExecutableDirectory() / "gui.json";
			}

			void loadWindows() {
				WindowSaveProvider saver(getGuiFile());
				saver.load();
				if (saver.hasWindow("ImageViewer"))
					createImageViewerWindow();
			}

			void saveWindows() {
				WindowSaveProvider saver(getGuiFile());
				if(m_imageViewerWindow)
					saver.addWindow("ImageViewer");
				saver.save();
			}

			void firstInitWindows() {
				createImageViewerWindow();
			}

			void createImageViewerWindow() {
				const auto panel = new ImageManagerPanel(m_project->getImageManager());
				panel->selectImageEventHandler([&](CE::ImageDecorator* imageDec, bool duplicate)
					{
						selectImage(imageDec, duplicate);
					});
				m_imageViewerWindow = new StdWindow(panel);
			}

			void createImageContentViewerWindow(CE::ImageDecorator* imageDec) {
				m_winManager.addWindow((new ImageContentViewerPanel(imageDec))->createStdWindow());
			}

			void selectImage(CE::ImageDecorator* imageDec, bool duplicate) {
				if (!imageDec->hasLoaded()) {
					delete m_messageWindow;
					m_messageWindow = CreateMessageWindow("The image has not loaded.");
					return;
				}
				if (!duplicate) {
					const auto existingWindow = m_winManager.findWindow<ImageContentViewerPanel>([&](ImageContentViewerPanel* panel)
						{
							return imageDec == panel->m_imageDec;
						});
					if (existingWindow) {
						existingWindow->focus();
						return;
					}
				}
				createImageContentViewerWindow(imageDec);
			}
		};

		StdWorkspace* m_workspace = nullptr;
		ImGuiID m_dockSpaceId;
	public:
		
		ProjectPanel(CE::Project* project)
			: AbstractPanel("Project: " + project->getDirectory().string() + "###project"), m_project(project)
		{
			m_workspace = new StdWorkspace(this);
		}

		~ProjectPanel() override {
			delete m_workspace;
		}

		StdWindow* createStdWindow() {
			const auto window = new StdWindow(this, ImGuiWindowFlags_MenuBar);
			window->getSize() = ImVec2(1000, 800);
			return window;
		}
	
	protected:
		void renderPanel() override {
			checkUnsavedState();
			ImGui::DockSpace(m_dockSpaceId = ImGui::GetID("ProjectDockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
			m_workspace->renderPanel();
		}

		void renderMenuBar() override {
			m_workspace->renderMenuBar();
		}

		void checkUnsavedState() const {
			m_window->addFlags(ImGuiWindowFlags_UnsavedDocument, m_project->getTransaction()->hasNewItems());
		}
	};
};
