#pragma once
#include "ImageDecorator.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Text.h"
#include "utilities/Helper.h"

namespace GUI
{
	class ImageContentViewerPanel : public AbstractPanel
	{
		CE::ImageDecorator* m_imageDec;
	public:
		ImageContentViewerPanel(CE::ImageDecorator* imageDec)
			: AbstractPanel("Image content viewer"), m_imageDec(imageDec)
		{}

	private:
		void renderPanel() override {
			auto data = m_imageDec->getImage()->getData();

			//ImGuiListClipper clipper();

			static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
			if (ImGui::BeginTable("content_table", 2, flags))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_None);
				ImGui::TableHeadersRow();

				ImGuiListClipper clipper;
				clipper.Begin(m_imageDec->getImage()->getSize());
				while (clipper.Step())
				{
					for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
					{
						using namespace Helper::String;
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						Text::Text(NumberToHex(row + 0x1000000)).show();
						ImGui::TableNextColumn();
						Text::Text(NumberToHex(data[row])).show();
					}
				}
				ImGui::EndTable();
			}
		}
	};
};