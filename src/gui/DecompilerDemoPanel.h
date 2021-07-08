#pragma once
#include <imgui_wrapper/Window.h>
#include <imgui_wrapper/widgets/code_editor/CodeEditor.h>
#include <Program.h>

#include "imgui_wrapper/controls/Button.h"
#include "imgui_wrapper/controls/Input.h"
#include "imgui_wrapper/controls/Text.h"
#include <imgui_wrapper/controls/TabBar.h>

#include "imgui_wrapper/controls/List.h"
#include "imgui_wrapper/controls/Tree.h"

namespace CE {
	class Project;
};

namespace GUI {
	class FunctionManagerPanel;
	class ImageViewerPanel;
	
	class DecompilerDemoPanel : public AbstractPanel
	{
		Widget::CodeEditor* m_decCodeEditor;
		Text::ColoredText m_asmParsingErrorText;
		Input::TextInput m_bytes_input;
		Text::ColoredText m_bytesParsingErrorText;
		Button::StdButton m_deassembly_btn;
		Button::StdButton m_assembly_btn;
		Button::StdButton m_decompile_btn;
		Text::ColoredText m_decInfoText;
		TabBar m_tabBar;
		
		StdListModel<int> m_testListModel;
		StdTreeModel<int> m_testTreeModel;

		StdWindow* m_functionManagerWindow = nullptr;
		StdWindow* m_imageViewerWindow = nullptr;
		PopupModalWindow* m_popupModalWin = nullptr;
		PopupBuiltinWindow* m_popupBuiltinWindow = nullptr;

		CE::Program* m_program;
		CE::Project* m_project;
	public:
		Widget::CodeEditor* m_asmCodeEditor;
		
		DecompilerDemoPanel();

		static StdWindow* GetStdWindow();
	protected:
		void renderPanel() override;

	private:

		void initProgram();

		void assembly(const std::string& hexBytesStr);

		void deassembly(const std::string& textCode);

		void decompile(const std::string& hexBytesStr);
	};
};