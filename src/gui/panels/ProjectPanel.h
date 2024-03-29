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
		
		CE::Project* m_project;
		Debugger* m_debugger = nullptr;
		PCodeEmulator* m_emulator = nullptr;
		StdWindow* m_imageViewerWindow = nullptr;
		StdWindow* m_funcViewerWindow = nullptr;
		StdWindow* m_dataTypeViewerWindow = nullptr;
		StdWindow* m_symbolViewerWindow = nullptr;
		StdWindow* m_debugAttachProcessWindow = nullptr;
		WindowManager m_imageContentWinManager;
		StdWindow* m_messageWindow = nullptr;
		ImGuiID m_dockSpaceId;

		// location history
		std::vector<std::pair<CE::ImageDecorator*, CE::Offset>> m_visitedLocationsHistory;
		int m_visitedLocationIdx = -1;
		bool m_lockLocationHistoryUpdate = false;
	public:
		std::pair<CE::ImageDecorator*, CE::Offset> m_curLocation = std::pair(nullptr, 0);
		
		ProjectPanel(CE::Project* project)
			: AbstractPanel("Project: " + project->getDirectory().string() + "###project"), m_project(project)
		{
			if (exists(getGuiFile())) {
				loadWindows();
			}
			else {
				firstInitWindows();
			}
		}

		~ProjectPanel() override {
			saveWindows();
			delete m_imageViewerWindow;
		}

		StdWindow* createStdWindow() {
			const auto window = new StdWindow(this, ImGuiWindowFlags_MenuBar);
			window->getSize() = ImVec2(1000, 800);
			return window;
		}
		
		void addVisitedLocation(CE::ImageDecorator* imageDec, CE::Offset offset) {
			if (m_lockLocationHistoryUpdate)
				return;
			m_visitedLocationIdx++;
			m_visitedLocationsHistory.resize(m_visitedLocationIdx + 2);
			m_visitedLocationsHistory[m_visitedLocationIdx] = m_curLocation;
			m_visitedLocationsHistory[m_visitedLocationIdx + 1] = std::pair(imageDec, offset);
		}

		PCodeEmulator* getEmulator(bool notStopped = true) const {
			if (!m_emulator) {
				if (!m_debugger || notStopped && !m_debugger->m_emulator->isWorking())
					return nullptr;
				return m_debugger->m_emulator;
			}
			if (notStopped && !m_emulator->isWorking())
				return nullptr;
			return m_emulator;
		}

		CE::Decompiler::FunctionPCodeGraph* m_prevFuncPCodeGraph = nullptr;
		void locationHandler(uint64_t delta = 0) {
			const auto emulator = getEmulator();
			const auto imageDec = emulator->m_imageDec;
			if (!imageDec)
				return;

			auto isNewLocation = delta > 100;
			auto isNewFuncGraph = false;
			if(emulator->m_curPCodeBlock) {
				const auto newFuncGraph = emulator->m_curPCodeBlock->m_funcPCodeGraph;
				isNewFuncGraph = m_prevFuncPCodeGraph != newFuncGraph;
				isNewLocation |= isNewFuncGraph;
				m_prevFuncPCodeGraph = newFuncGraph;
			}
			if (isNewLocation) {
				selectImage(imageDec, false);
			}

			const auto window = getImageContentViewerWindow(imageDec);
			if (!window)
				return;
			const auto panel = dynamic_cast<ImageContentViewerPanel*>(window->getPanel());
			if (!panel)
				return;

			if (isNewLocation) {
				try {
					panel->goToOffset(emulator->m_offset.getByteOffset());
				} catch(WarningException&) {}
			}

			if (panel->m_curDecGraph) {
				if(isNewFuncGraph) {
					panel->decompile(m_prevFuncPCodeGraph);
				}
				emulator->m_curDecGraph = panel->m_curDecGraph;
			}
		}

		void createEmulator(CE::ImageDecorator* imageDec, CE::ComplexOffset startOffset, PCodeEmulator::PCodeStepWidth stepWidth) {
			delete m_emulator;
			m_emulator = new PCodeEmulator(imageDec->getAddressSpace(), imageDec, startOffset);
			m_emulator->m_stepWidth = stepWidth;
			m_emulator->locationHandler([&](uint64_t delta)
				{
					locationHandler(delta);
				});
			m_emulator->sync();
		}
	
	protected:
		void renderPanel() override;

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
				if (!m_debugger) {
					if (ImGui::MenuItem("Attach Process", nullptr)) {
						delete m_debugAttachProcessWindow;
						const auto panel = new DebuggerAttachProcessPanel(m_project);
						panel->selectProcessEventHandler([&, panel]()
							{
								delete m_emulator;
								m_emulator = nullptr;
								m_debugger = new Debugger(m_project, panel->m_debugSession, panel->m_selectedParentAddrSpace);
								m_debugger->m_emulator->locationHandler([&](uint64_t delta)
									{
										locationHandler(delta);
									});
							});
						m_debugAttachProcessWindow = new StdWindow(panel);
					}
				}

				if (const auto emulator = getEmulator(false)) {
					emulator->renderDebugMenu();
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

		void checkUnsavedState() const {
			m_window->addFlags(ImGuiWindowFlags_UnsavedDocument, m_project->getTransaction()->hasNewItems());
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
			if (m_imageViewerWindow)
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
			m_imageContentWinManager.addWindow((new ImageContentViewerPanel(imageDec, this))->createStdWindow());
		}

		void selectImage(CE::ImageDecorator* imageDec, bool duplicate) {
			if (!imageDec->hasLoaded()) {
				delete m_messageWindow;
				m_messageWindow = CreateMessageWindow("The image has not loaded.");
				return;
			}
			if (!duplicate) {
				if (const auto existingWindow = getImageContentViewerWindow(imageDec)) {
					existingWindow->focus();
					return;
				}
			}
			createImageContentViewerWindow(imageDec);
		}

		StdWindow* getImageContentViewerWindow(CE::ImageDecorator* imageDec) {
			return m_imageContentWinManager.findWindow<ImageContentViewerPanel>([&](ImageContentViewerPanel* panel)
				{
					return imageDec == panel->m_imageDec;
				});
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
};
