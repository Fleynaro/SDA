#pragma once
#include <functional>
#include <list>
#include "Control.h"

namespace GUI
{
	struct TabItem
	{
		std::string m_name;
		std::function<void()> m_drawCall;
		
		TabItem(const std::string& name, std::function<void()> drawCall)
			: m_name(name), m_drawCall(drawCall)
		{}
	};
	
	class TabBar
		: public Control,
		public Attribute::Name
	{
		std::list<TabItem> m_tabItems;
		std::string m_selectedTabItemName;
	public:
		TabBar(const std::string& name = "#tabs")
			: Attribute::Name(name)
		{}

		void selectTabItem(const std::string& name)
		{
			m_selectedTabItemName = name;
		}

		void present(const std::list<TabItem>& tabItems)
		{
			m_tabItems = tabItems;
			show();
		}
	protected:
		void renderControl() override
		{
			if (ImGui::BeginTabBar(getName().c_str())) {
				for (auto tabItem : m_tabItems)
				{
					ImGuiTabItemFlags_ flags = ImGuiTabItemFlags_None;
					if (tabItem.m_name == m_selectedTabItemName)
						flags = ImGuiTabItemFlags_SetSelected;
					if (ImGui::BeginTabItem(tabItem.m_name.c_str(), nullptr, flags)) {
						tabItem.m_drawCall();
						ImGui::EndTabItem();
					}
				}
				m_selectedTabItemName = "";
				ImGui::EndTabBar();
			}
		}
	};
};