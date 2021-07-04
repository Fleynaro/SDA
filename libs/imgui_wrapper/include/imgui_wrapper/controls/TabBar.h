#pragma once
#include "Control.h"

namespace GUI
{
	/*class TabBar
	{
		std::string* m_selectedTabItemName;
		bool m_isOpen;
	public:
		TabBar(std::string* selectedTabItemName, const char* tabBarName = "#tabs")
			: m_selectedTabItemName(selectedTabItemName)
		{
			m_isOpen = ImGui::BeginTabBar(tabBarName);
		}

		~TabBar()
		{
			ImGui::EndTabBar();
		}

		bool isOpen()
		{
			return m_isOpen;
		}
	};

	class TabItem
	{
		TabBar* m_tabBar;
		ImGuiTabItemFlags_ m_flags = ImGuiTabItemFlags_None;
		bool m_isOpen;
	public:
		TabItem(TabBar* tabBar, const std::string& name)
			: m_tabBar(tabBar)
		{
			m_isOpen = ImGui::BeginTabItem(name.c_str(), nullptr, m_flags);
		}

		~TabItem()
		{
			ImGui::EndTabItem();
		}

		bool isOpen()
		{
			return m_isOpen;
		}
	};*/
	
	class TabItem2 :
		public Control,
		public Attribute::Id,
		public Attribute::Name,
		public Attribute::Flags<
		ImGuiTabItemFlags,
		ImGuiTabItemFlags_None
		>
	{
		bool m_isOpened = false;
		bool m_isSelected = false;
	public:
		TabItem2(const std::string& name, ImGuiTabItemFlags flags = ImGuiTabItemFlags_None)
			: Attribute::Name(name), Attribute::Flags<ImGuiTabItemFlags, ImGuiTabItemFlags_None>(flags)
		{}

		bool present() {
			Control::show();
			return isOpened();
		}

		void select() {
			addFlags(ImGuiTabItemFlags_SetSelected, true);
			m_isSelected = true;
		}

		bool isOpened() {
			return m_isOpened;
		}

	protected:
		void renderControl() override {
			pushIdParam();

			m_isOpened = ImGui::BeginTabItem(getName().c_str(), nullptr, getFlags());
			if (m_isSelected) {
				addFlags(ImGuiTabItemFlags_SetSelected, false);
				m_isSelected = false;
			}

			popIdParam();
		}
	};
};