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
		CE::Decompiler::FunctionPCodeGraph* m_functionPCodeGraph;
	public:
		FuncGraphViewerPanel(CodeSectionController* codeSectionController, AbstractInstructionViewer* instructionViewer)
			: AbstractPanel("Function graph viewer"), m_codeSectionController(codeSectionController), m_instructionViewer(instructionViewer)
		{}

		void setFuncGraph(CE::Decompiler::FunctionPCodeGraph* functionPCodeGraph) {
			m_functionPCodeGraph = functionPCodeGraph;
		}
	
	private:
		void renderPanel() override {
			Text::Text("text").show();
		}
	};
};