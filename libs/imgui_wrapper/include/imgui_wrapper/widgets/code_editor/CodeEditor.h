#pragma once
#include "../../GUI.h"
#include "TextEditor.h"

namespace GUI::Widget
{
	class CodeEditor :
		public Control,
		public Attribute::Name
	{
		ImVec2 m_size;
	public:
		CodeEditor(const std::string& name, ImVec2 size)
			: Attribute::Name(name), m_size(size)
		{}

		TextEditor& getEditor() {
			return m_editor;
		}

		ImVec2& getSize() {
			return m_size;
		}

		/*void setSize(ImVec2 size) {
			m_size = size;
		}*/
	protected:
		TextEditor m_editor;

		void renderControl() override {
			m_editor.Render(getName().c_str(), m_size);
		}
	};
};