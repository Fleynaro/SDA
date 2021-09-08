#pragma once
#include "ImageManagerPanel.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "controllers/SymbolManagerController.h"
#include "editors/SymbolEditorPanel.h"

namespace GUI
{
	class SymbolListViewGrouping
		: public ListViewGrouping<CE::Symbol::AbstractSymbol*>
	{
	public:
		using ListViewGrouping<CE::Symbol::AbstractSymbol*>::ListViewGrouping;

	protected:
		bool groupBy(CE::Symbol::AbstractSymbol* const& symbol1, CE::Symbol::AbstractSymbol* const& symbol2) override {
			const auto gvar1 = dynamic_cast<CE::Symbol::GlobalVarSymbol*>(symbol1);
			const auto gvar2 = dynamic_cast<CE::Symbol::GlobalVarSymbol*>(symbol2);
			return gvar1->m_globalSymbolTable->m_imageDec > gvar2->m_globalSymbolTable->m_imageDec;
		}

		bool renderGroupTop(CE::Symbol::AbstractSymbol* const& symbol, int group_n) override {
			const auto gvar = dynamic_cast<CE::Symbol::GlobalVarSymbol*>(symbol);
			return ImGui::CollapsingHeader(gvar->m_globalSymbolTable->m_imageDec->getName(), ImGuiTreeNodeFlags_DefaultOpen);
		}
	};

	class SymbolManagerPanel : public AbstractPanel
	{
		class SymbolListView : public SymbolListViewGrouping
		{
			class SymbolContextPanel : public AbstractPanel
			{
				SymbolManagerPanel* m_symManagerPanel;
				CE::Symbol::GlobalVarSymbol* m_symbol;
				ImVec2 m_winPos;
			public:
				SymbolContextPanel(CE::Symbol::GlobalVarSymbol* symbol, SymbolManagerPanel* symManagerPanel, ImVec2 winPos)
					: m_symbol(symbol), m_symManagerPanel(symManagerPanel), m_winPos(winPos)
				{}

			private:
				void renderPanel() override {
					if (ImGui::MenuItem("Edit")) {
						delete m_symManagerPanel->m_symbolEditor;
						m_symManagerPanel->m_symbolEditor = new StdWindow(new SymbolEditorPanel(m_symbol));
					}
				}
			};

			SymbolManagerPanel* m_symManagerPanel;
		public:
			SymbolListView(StdListView<CE::Symbol::AbstractSymbol*>* listView, SymbolManagerPanel* symManagerPanel)
				: SymbolListViewGrouping(listView), m_symManagerPanel(symManagerPanel)
			{}

		private:
			void renderItem(const std::string& text, CE::Symbol::AbstractSymbol* const& symbol, int n) override {
				const auto gvar = dynamic_cast<CE::Symbol::GlobalVarSymbol*>(symbol);
				SymbolListViewGrouping::renderItem(text, symbol, n);
				const auto events = GenericEvents(true);
				if (events.isClickedByLeftMouseBtn()) {
					if (m_symManagerPanel->m_selectSymbolEventHandler.isInit())
						m_symManagerPanel->m_selectSymbolEventHandler(symbol);
				}
				if (events.isClickedByRightMouseBtn()) {
					delete m_symManagerPanel->m_symbolContextWindow;
					m_symManagerPanel->m_symbolContextWindow = new PopupContextWindow(new SymbolContextPanel(gvar, m_symManagerPanel, GetLeftBottom()));
					m_symManagerPanel->m_symbolContextWindow->open();
				}
			}
		};

		class SymbolFilter : public Control
		{
			SymbolManagerPanel* m_panel;
			Input::TextInput m_search_input;
		public:
			bool m_isUpdated = false;

			SymbolFilter(SymbolManagerPanel* panel)
				: m_panel(panel)
			{}
		protected:
			void renderControl() override {
				auto& filter = m_panel->m_controller.m_filter;

				m_isUpdated = false;
				if (m_search_input.isTextEntering()) {
					filter.m_name = m_search_input.getInputText();
					m_isUpdated = true;
				}

				m_search_input.show();
			}
		};

		CE::SymbolManager* m_manager;
		SymbolManagerController m_controller;
		SymbolFilter m_filterControl;
		SymbolListView* m_listView = nullptr;
		PopupContextWindow* m_symbolContextWindow = nullptr;
		StdWindow* m_symbolEditor = nullptr;
		EventHandler<CE::Symbol::AbstractSymbol*> m_selectSymbolEventHandler;
	public:
		SymbolManagerPanel(CE::SymbolManager* manager)
			: AbstractPanel("Global Var Manager"), m_manager(manager), m_controller(manager), m_filterControl(this)
		{
			/*m_listView = new TableListView(&m_controller.m_listModel, {
				ColInfo("Symbol")
			});*/
			const auto listView = new StdListView(&m_controller.m_listModel);
			m_listView = new SymbolListView(listView, this);
			m_controller.m_maxItemsCount = 50;
			m_controller.update();
		}

		~SymbolManagerPanel() override {
			delete m_listView;
		}

		void selectSymbolEventHandler(const std::function<void(CE::Symbol::AbstractSymbol*)>& eventHandler) {
			m_selectSymbolEventHandler = eventHandler;
		}

	protected:
		void renderPanel() override {
			m_filterControl.show();
			if (m_filterControl.m_isUpdated) {
				m_controller.update();
			}

			m_listView->show();
			Show(m_symbolContextWindow);
			Show(m_symbolEditor);
		}
	};
};
