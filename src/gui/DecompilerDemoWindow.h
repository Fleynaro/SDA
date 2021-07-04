#pragma once
#include <imgui_wrapper/Window.h>
#include <imgui_wrapper/widgets/code_editor/CodeEditor.h>
#include <Program.h>

#include "imgui_wrapper/controls/Button.h"
#include "imgui_wrapper/controls/Input.h"
#include "imgui_wrapper/controls/Text.h"

namespace CE {
	class Project;
};

namespace GUI {
	class DecompilerDemoWindow : public Window
	{
		Widget::CodeEditor* m_asmCodeEditor;
		Widget::CodeEditor* m_decCodeEditor;
		Text::ColoredText m_asmParsingErrorText;
		Input::TextInput m_bytes_input;
		Text::ColoredText m_bytesParsingErrorText;
		Button::StdButton m_deassembly_btn;
		Button::StdButton m_assembly_btn;
		Button::StdButton m_decompile_btn;
		Text::ColoredText m_decInfoText;

		CE::Program* m_program;
		CE::Project* m_project;
	public:
		DecompilerDemoWindow()
			: Window("Decompiler")
		{
			// Window params
			setFlags(ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
			setFullscreen(true);

			// Controls
			m_asmCodeEditor = new Widget::CodeEditor("assembler code", ImVec2(200.0f, 300.0f));
			m_asmCodeEditor->getEditor().SetLanguageDefinition(TextEditor::LanguageDefinition::C());
			m_asmCodeEditor->getEditor().SetText(
				"mov eax, 0x10\n"
				"mov ebx, 0x20\n"
				"add eax, ebx\n"
				"mov [rsp], eax"
			);

			m_decCodeEditor = new Widget::CodeEditor("decompiled code", ImVec2(200.0f, 400.0f));
			m_decCodeEditor->getEditor().SetLanguageDefinition(TextEditor::LanguageDefinition::C());
			m_decCodeEditor->getEditor().SetReadOnly(true);

			m_asmParsingErrorText.setColor(ColorRGBA(0xFF0000FF));
			m_asmParsingErrorText.setDisplay(false);
			m_bytesParsingErrorText.setColor(ColorRGBA(0xFF0000FF));
			m_bytesParsingErrorText.setDisplay(false);
			m_decInfoText.setColor(ColorRGBA(0xFF0000FF));
			m_decInfoText.setDisplay(false);
			m_bytes_input = Input::TextInput();
			m_deassembly_btn = Button::StdButton("deassembly");
			m_assembly_btn = Button::StdButton("assembly");
			m_decompile_btn = Button::StdButton("decompile");

			initProgram();
		}

	protected:

		void renderWindow() override {
			m_asmCodeEditor->getSize() = getSize();
			m_asmCodeEditor->getSize().y *= 0.6f;
			m_decCodeEditor->getSize() = getSize();

			if (ImGui::BeginTabBar("#tabs"))
			{
				ImGuiTabItemFlags_ tabFlag_disassembler = ImGuiTabItemFlags_None;
				ImGuiTabItemFlags_ tabFlag_decompiler = ImGuiTabItemFlags_None;

				if (ImGui::BeginTabItem("disassembler", nullptr, tabFlag_disassembler))
				{
					{
						Text::Text("Here write your assembler code:").show();
						m_asmCodeEditor->show();
						m_asmParsingErrorText.show();

						NewLine();
						if (m_deassembly_btn.present()) {
							m_asmParsingErrorText.setDisplay(false);

							auto textCode = m_asmCodeEditor->getEditor().GetText();
							if (!textCode.empty()) {
								deassembly(textCode);
							}
						}

						SameLine();
						if (m_assembly_btn.present()) {
							m_bytesParsingErrorText.setDisplay(false);
							if (!m_bytes_input.getInputText().empty()) {
								assembly(m_bytes_input.getInputText());
							}
						}
					}

					{
						NewLine();
						Separator();
						Text::Text("The machine code is presented as bytes in hexadecimal format:").show();
						m_bytes_input.show();
						m_bytesParsingErrorText.show();
						NewLine();
						if (m_decompile_btn.present()) {
							m_bytesParsingErrorText.setDisplay(false);

							if (!m_bytes_input.getInputText().empty()) {
								tabFlag_decompiler = ImGuiTabItemFlags_SetSelected;
								decompile(m_bytes_input.getInputText());
							}
							else {
								m_bytesParsingErrorText.setDisplay(true);
								m_bytesParsingErrorText.setText("Please, enter hex bytes (format: 66 89 51 02)");
							}
						}
					}
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("decompiler", nullptr, tabFlag_decompiler))
				{
					{
						NewLine();
						Separator();
						Text::Text("Here the decompiled code is presented:").show();
						m_decCodeEditor->show();
						m_decInfoText.show();
					}
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}

	private:

		void initProgram();

		void assembly(const std::string& hexBytesStr);

		void deassembly(const std::string& textCode);

		void decompile(const std::string& hexBytesStr);
	};
};