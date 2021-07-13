#pragma once


namespace GUI
{
	static void RenderAddress(uint64_t offset) {
		using namespace Helper::String;
		const auto offsetStr = NumberToHex(offset + (static_cast<uint64_t>(1) << 63)).substr(1);
		Text::Text("base + " + offsetStr).show();
	}

	class FuncGraphViewerPanel : public AbstractPanel
	{
		CodeSectionController* m_codeSectionController;
		AbstractInstructionViewer* m_instructionViewer;
		CE::Decompiler::FunctionPCodeGraph* m_funcPCodeGraph = nullptr;
	public:
		FuncGraphViewerPanel(CodeSectionController* codeSectionController, AbstractInstructionViewer* instructionViewer)
			: AbstractPanel("Function graph viewer"), m_codeSectionController(codeSectionController), m_instructionViewer(instructionViewer)
		{}

		void setFuncGraph(CE::Decompiler::FunctionPCodeGraph* functionPCodeGraph) {
			m_funcPCodeGraph = functionPCodeGraph;
		}
	
	private:
		void renderPanel() override {
			Text::Text("text " + std::to_string(m_funcPCodeGraph->getStartBlock()->getMinOffset().getByteOffset())).show();

			static ImVec2 offset(30.0f, 30.0f);
			
			ImGui::PushID(1);
			ImGui::BeginGroup();

			{
				ImGui::InvisibleButton("##empty", ImGui::GetContentRegionAvail());
				if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					offset.x += ImGui::GetIO().MouseDelta.x;
					offset.y += ImGui::GetIO().MouseDelta.y;
				}
				const ImVec2 p0 = ImGui::GetItemRectMin();
				const ImVec2 p1 = ImGui::GetItemRectMax();
				const char* text_str = "Line 1 hello\nLine 2 clip me!";
				const ImVec2 text_pos = ImVec2(p0.x + offset.x, p0.y + offset.y);
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				
				ImGui::PushClipRect(p0, p1, true);
				draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
				draw_list->AddText(text_pos, IM_COL32_WHITE, text_str);
				ImGui::PopClipRect();
			}

			ImGui::EndGroup();
			ImGui::PopID();
		}
	};
};