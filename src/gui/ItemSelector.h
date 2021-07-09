#pragma once
#include <set>
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/Input.h"

namespace GUI
{
	template<typename T>
	class ItemSelector : public Control
	{
	protected:
		Input::TextInput m_search_input;
		PopupBuiltinWindow* m_popupBuiltinWindow;
		bool m_focusOnSearchInput = false;
		int m_prevSelItemsCount = 0;
	public:
		bool m_isUpdated = false;
		std::set<T*>* m_selectedItems = nullptr;

		ItemSelector()
		{
			m_popupBuiltinWindow = new PopupBuiltinWindow(new StdPanel([&]()
				{
					renderPopupBuiltinWindow();
				}));
		}

		~ItemSelector() override
		{
			delete m_popupBuiltinWindow;
		}

	protected:
		virtual void renderPopupBuiltinWindow() = 0;

		virtual void textEntering(const std::string& text) = 0;

		void renderControl() override {
			m_isUpdated = false;
			m_popupBuiltinWindow->show();

			const auto selItemsCount = static_cast<int>(m_selectedItems->size());
			if (m_prevSelItemsCount != selItemsCount) {
				m_isUpdated = true;
				//m_focusOnSearchInput = true;
				m_prevSelItemsCount = selItemsCount;
			}

			if (m_focusOnSearchInput && m_popupBuiltinWindow->isOpened()) {
				ImGui::SetKeyboardFocusHere();
				m_focusOnSearchInput = false;
			}
			m_search_input.show();
			m_popupBuiltinWindow->placeAfterItem();

			if (m_search_input.isTextEntering() || m_search_input.isClickedByLeftMouseBtn()) {
				m_focusOnSearchInput = true;
				textEntering(m_search_input.getInputText());
				m_popupBuiltinWindow->open();
			}
		}
	};
};