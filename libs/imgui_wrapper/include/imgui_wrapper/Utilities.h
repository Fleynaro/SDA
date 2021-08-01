#pragma once
#include "controls/Control.h"

namespace GUI
{
	static ImVec4 ToImGuiColor(ColorRGBA color) {
		ColorComponent A = color & 0xFF;
		ColorComponent R = color >> 24 & 0xFF;
		ColorComponent G = color >> 16 & 0xFF;
		ColorComponent B = color >> 8 & 0xFF;
		return ImColor(R, G, B, A);
	}

	static ImU32 ToImGuiColorU32(ColorRGBA color) {
		return ImGui::GetColorU32(ToImGuiColor(color));
	}
	
	static void DrawBorder(ColorRGBA color, float padding = 1.0) {
		auto min = ImGui::GetItemRectMin();
		auto max = ImGui::GetItemRectMax();
		min.x -= padding;
		min.y -= padding;
		max.x += padding;
		max.y += padding;
		ImGui::GetWindowDrawList()->AddRect(min, max, ImGui::GetColorU32(ToImGuiColor(color)));
	}

	static void SameLine(float spacing = -1.f) {
		ImGui::SameLine(0.f, spacing);
	}

	static void Spacing() {
		ImGui::Spacing();
	}

	static void NewLine() {
		ImGui::NewLine();
	}

	static void Separator() {
		ImGui::Separator();
	}

	class MultiLineGroup
	{
	public:
		struct Group
		{
			void* m_obj = nullptr;
			GenericEvents m_events;
			std::list<ImRect> m_rects;

			void update() {
				auto events = GenericEvents(true);
				m_events.join(&events);
				m_rects.push_back(ImGui::GetCurrentWindowRead()->DC.LastItemRect);
			}
		};
	private:
		std::list<Group> m_groups;
	public:
		MultiLineGroup()
		{}

		void beginGroup(void* obj) {
			ImGui::BeginGroup();
			Group group;
			group.m_obj = obj;
			m_groups.push_back(group);
		}

		Group endGroup() {
			ImGui::EndGroup();
			SameLine(0.0f);
			auto group = *m_groups.rbegin();
			group.update();
			m_groups.pop_back();
			return group;
		}

		// used when NewLine()/... called
		void beginSeparator() {
			for (auto it = m_groups.rbegin(); it != m_groups.rend(); ++it) {
				ImGui::EndGroup();
				SameLine(0.0f);
				it->update();
			}
		}

		void endSeparator() {
			for (auto it = m_groups.begin(); it != m_groups.end(); ++it) {
				ImGui::BeginGroup();
			}
		}
	};

	static void HighlightItem(ColorRGBA color) {
		ImGui::GetWindowDrawList()->AddRectFilled(
			ImGui::GetItemRectMin() - ImVec2(2.0f, 1.0f), ImGui::GetItemRectMax() + ImVec2(2.0f, 1.0f), ToImGuiColorU32(color));
		ImGui::SetCursorScreenPos(ImGui::GetItemRectMin());
	}

	static void FrameItem(ColorRGBA color) {
		ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin() - ImVec2(1.0f, 1.0f), ImGui::GetItemRectMax() + ImVec2(1.0f, 1.0f), ToImGuiColorU32(color));
	}

	template<typename T>
	class HighlightedItem
	{
		T* m_item = nullptr;
		std::list<ImRect> m_rects;
		ColorRGBA m_color = -1;
	public:
		void set(T* item, const std::list<ImRect>& rects, ColorRGBA color) {
			m_item = item;
			m_rects = rects;
			m_color = color;
		}

		void clear() {
			m_item = nullptr;
			m_rects = {};
			m_color = -1;
		}

		bool draw(T* item) {
			if (item != m_item)
				return false;
			for (const auto& rect : m_rects) {
				ImGui::GetWindowDrawList()->AddRectFilled(
					rect.Min - ImVec2(2.0f, 1.0f), rect.Max + ImVec2(2.0f, 1.0f), ToImGuiColorU32(m_color));
			}
			return true;
		}
	};
};