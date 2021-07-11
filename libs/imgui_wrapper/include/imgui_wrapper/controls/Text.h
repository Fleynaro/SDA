#pragma once
#include "Control.h"
#include "imgui_wrapper/Events.h"
#include <imgui_wrapper/Utilities.h>

namespace GUI::Text
{
	class Text
		: public Control,
		public GenericEvents,
		public Attribute::Size,
		public Attribute::Font
	{
	public:
		Text(const std::string& text = "")
			: m_text(text)
		{}

		void setText(const std::string& text) {
			m_text = text;
		}

		const std::string& getText() {
			return m_text;
		}
	protected:
		std::string m_text;

		void renderControl() override {
			pushWidthParam();
			pushFontParam();

			renderText();
			processGenericEvents();

			popFontParam();
			popWidthParam();
		}

		virtual void renderText() {
			ImGui::Text(getText().c_str());
		}
	};

	class BulletText : public Text
	{
	public:
		BulletText(const std::string& text = "")
			: Text(text)
		{}

	protected:
		void renderText() override {
			ImGui::BulletText(m_text.c_str());
		}
	};

	class ColoredText : public Text
	{
		ColorRGBA m_color;
	public:
		ColoredText(const std::string& text = "", ColorRGBA color = -1)
			: Text(text), m_color(color)
		{}

		void setColor(ColorRGBA color) {
			m_color = color;
		}
	protected:
		void renderText() override {
			ImGui::TextColored(
				ToImGuiColor(m_color),
				m_text.c_str()
			);
		}
	};
};