#pragma once
#include "ImGuiFileDialog.h"
#include "imgui_wrapper/controls/Button.h"
#include "imgui_wrapper/controls/Control.h"
#include "imgui_wrapper/controls/Input.h"
#include <filesystem>

namespace GUI::Widget
{
	class FileDialog :
		public Control,
		public Attribute::Name,
		public Attribute::Size
	{
		Input::TextInput m_pathInput;
		std::string m_filters;
	public:
		FileDialog(const std::string& name = "Choose Path", const std::string& filters = "dir")
			: Attribute::Name(name), Attribute::Size(ImVec2(600, 500)), m_filters(filters)
		{}

		void setPath(const std::filesystem::path& path) {
			m_pathInput.setInputText(path.string());
		}

		std::filesystem::path getPath() {
			return m_pathInput.getInputText();
		}
	protected:
		
		void renderControl() override {
			m_pathInput.show();
			SameLine();
			if (Button::StdButton("Open").present()) {
				const auto vFilters = m_filters == "dir" ? nullptr : m_filters.c_str();
				ImGuiFileDialog::Instance()->OpenModal("fd", getName(), vFilters, ".", 1, nullptr, ImGuiFileDialogFlags_Default);
			}
			
			if (ImGuiFileDialog::Instance()->Display("fd", ImGuiWindowFlags_None, getSize())) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
					m_pathInput.setInputText(filePathName);
				}
				ImGuiFileDialog::Instance()->Close();
			}
		}
	};
};