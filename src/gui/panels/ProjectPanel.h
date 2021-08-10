#pragma once
#include "FunctionManagerPanel.h"
#include "ImageContentViewerPanel.h"
#include "ImageManagerPanel.h"
#include "Program.h"
#include "Project.h"
#include "SymbolManagerPanel.h"
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
			StdWindow* m_funcViewerWindow = nullptr;
			StdWindow* m_dataTypeViewerWindow = nullptr;
			StdWindow* m_symbolViewerWindow = nullptr;
			StdWindow* m_debugAttachProcessWindow = nullptr;
			WindowManager m_imageContentWinManager;
			StdWindow* m_messageWindow = nullptr;
			Debugger* m_debugger = nullptr;
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
				m_imageContentWinManager.m_dockSpaceId = m_projectPanel->m_dockSpaceId;
				m_imageContentWinManager.show();
				Show(m_imageViewerWindow);
				Show(m_funcViewerWindow);
				Show(m_dataTypeViewerWindow);
				Show(m_symbolViewerWindow);
				Show(m_debugAttachProcessWindow);
				Show(m_messageWindow);

				if (m_debugger) {
					m_debugger->show();
				}
			}

			void renderMenuBar() override {
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save", nullptr, false, m_project->getTransaction()->hasNewItems())) {
						m_project->getTransaction()->commit();
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Debug"))
				{
					if (ImGui::MenuItem("Attach Process", nullptr)) {
						delete m_debugAttachProcessWindow;
						const auto panel = new DebuggerAttachProcessPanel;
						panel->selectProcessEventHandler([&, panel]()
							{
								delete m_debugger;
								m_debugger = new Debugger(m_project, panel->m_debugSession);
							});
						m_debugAttachProcessWindow = new StdWindow(panel);
					}
				}

				if (ImGui::BeginMenu("View"))
				{
					if (ImGui::MenuItem("Image Viewer", nullptr, m_imageViewerWindow != nullptr)) {
						if (m_imageViewerWindow)
							m_imageViewerWindow->close();
						else createImageViewerWindow();
					}
					if (ImGui::MenuItem("Function Viewer", nullptr, m_funcViewerWindow != nullptr)) {
						if (m_funcViewerWindow)
							m_funcViewerWindow->close();
						else createFuncViewerWindow();
					}
					if (ImGui::MenuItem("Data Type Viewer", nullptr, m_dataTypeViewerWindow != nullptr)) {
						if (m_dataTypeViewerWindow)
							m_dataTypeViewerWindow->close();
						else createDataTypeViewerWindow();
					}
					if (ImGui::MenuItem("Global Var Viewer", nullptr, m_symbolViewerWindow != nullptr)) {
						if (m_symbolViewerWindow)
							m_symbolViewerWindow->close();
						else createSymbolViewerWindow();
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
				if (saver.hasWindow("FunctionViewer"))
					createFuncViewerWindow();
				if (saver.hasWindow("DataTypeViewer"))
					createDataTypeViewerWindow();
				if (saver.hasWindow("SymbolViewer"))
					createSymbolViewerWindow();
			}

			void saveWindows() {
				WindowSaveProvider saver(getGuiFile());
				if(m_imageViewerWindow)
					saver.addWindow("ImageViewer");
				if (m_funcViewerWindow)
					saver.addWindow("FunctionViewer");
				if (m_dataTypeViewerWindow)
					saver.addWindow("DataTypeViewer");
				if (m_symbolViewerWindow)
					saver.addWindow("SymbolViewer");
				saver.save();
			}

			void firstInitWindows() {
				createImageViewerWindow();
				createFuncViewerWindow();
				createDataTypeViewerWindow();
				createSymbolViewerWindow();
			}

			void createImageViewerWindow() {
				const auto panel = new ImageManagerPanel(m_project->getImageManager());
				panel->selectImageEventHandler([&](CE::ImageDecorator* imageDec, bool duplicate)
					{
						selectImage(imageDec, duplicate);
					});
				m_imageViewerWindow = new StdWindow(panel);
			}

			void createFuncViewerWindow() {
				const auto panel = new FunctionManagerPanel(m_project->getFunctionManager());
				panel->selectFuncEventHandler([&](CE::Function* func)
					{
						selectFunction(func);
					});
				m_funcViewerWindow = new StdWindow(panel);
			}

			void createDataTypeViewerWindow() {
				const auto panel = new DataTypeManagerPanel(m_project->getTypeManager());
				m_dataTypeViewerWindow = new StdWindow(panel);
			}

			void createSymbolViewerWindow() {
				const auto panel = new SymbolManagerPanel(m_project->getSymbolManager());
				panel->selectSymbolEventHandler([&](CE::Symbol::AbstractSymbol* symbol)
					{
						selectSymbol(symbol);
					});
				m_symbolViewerWindow = new StdWindow(panel);
			}

			void createImageContentViewerWindow(CE::ImageDecorator* imageDec) {
				m_imageContentWinManager.addWindow((new ImageContentViewerPanel(imageDec))->createStdWindow());
			}

			void selectImage(CE::ImageDecorator* imageDec, bool duplicate) {
				if (!imageDec->hasLoaded()) {
					delete m_messageWindow;
					m_messageWindow = CreateMessageWindow("The image has not loaded.");
					return;
				}
				if (!duplicate) {
					const auto existingWindow = m_imageContentWinManager.findWindow<ImageContentViewerPanel>([&](ImageContentViewerPanel* panel)
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

			void selectFunction(CE::Function* func) {
				selectImage(func->getImage(), false);
				// todo: go to func
			}

			void selectSymbol(CE::Symbol::AbstractSymbol* symbol) {
				const auto gvar = dynamic_cast<CE::Symbol::GlobalVarSymbol*>(symbol);
				selectImage(gvar->m_globalSymbolTable->m_imageDec, false);
				// todo: go to gvar
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
