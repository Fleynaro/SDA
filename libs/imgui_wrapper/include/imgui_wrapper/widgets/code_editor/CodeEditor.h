#pragma once
#include "imgui_wrapper/controls/Control.h"
#include "TextEditor.h"

namespace GUI::Widget
{
	class CodeEditor :
		public Control,
		public Attribute::Name,
		public Attribute::Size
	{
	public:
		CodeEditor(const std::string& name, ImVec2 size)
			: Attribute::Name(name), Attribute::Size(size)
		{}

		TextEditor& getEditor() {
			return m_editor;
		}
	protected:
		TextEditor m_editor;

		void renderControl() override {
			m_editor.Render(getName().c_str(), m_size);
		}
	};
};