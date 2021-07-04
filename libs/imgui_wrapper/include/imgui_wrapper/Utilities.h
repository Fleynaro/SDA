#pragma once
#include "controls/Control.h"

namespace GUI
{
	static inline ImVec4 ToImGuiColor(ColorRGBA color) {
		ColorComponent A = color & 0xFF;
		ColorComponent R = color >> 24 & 0xFF;
		ColorComponent G = color >> 16 & 0xFF;
		ColorComponent B = color >> 8 & 0xFF;
		return ImColor(R, G, B, A);
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

};