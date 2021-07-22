#pragma once
#include "Control.h"
#include "imgui_wrapper/Events.h"
#include "imgui_wrapper/Utilities.h"

namespace GUI::Input
{
	class AbstractInput
		: public Control,
		public GenericEvents,
		public Attribute::Id,
		public Attribute::Name,
		public Attribute::Flags<
		ImGuiInputTextFlags,
		ImGuiInputTextFlags_None
		>
	{
		ColorRGBA m_borderColor = 0x0;
		uint64_t m_borderHideTime = 0;
		bool m_focus = false;
	public:
		AbstractInput(const std::string& name)
			: Attribute::Name(name)
		{}

		void setReadOnly(bool toggle) {
			addFlags(ImGuiInputTextFlags_ReadOnly, toggle);
		}

		void showBorder(ColorRGBA color, int ms = 0) {
			m_borderColor = color;
			if (ms) {
				m_borderHideTime = GetTimeInMs() + ms;
			}
			else {
				m_borderHideTime = 0;
			}
		}

		void focus() {
			m_focus = true;
		}

		void hideBorder() {
			m_borderColor = 0x0;
		}

		/*void onExceptionOccured(const Exception& exception) override {
			showBorder(0xFF0000AA, 3000);
		}*/
	protected:
		void drawInputBorder() {
			if (m_borderColor != 0x0 && (m_borderHideTime == 0 || GetTimeInMs() < m_borderHideTime)) {
				DrawBorder(m_borderColor);
			}
		}

		void renderControl() override {
			pushIdParam();
			if(m_focus) {
				ImGui::SetKeyboardFocusHere();
				m_focus = false;
			}
			renderInput();
			drawInputBorder();
			popIdParam();
		}

		virtual void renderInput() = 0;
	};

	class TextInput
		: public AbstractInput,
		public Attribute::Size,
		public Attribute::Font
	{
		std::string m_inputValue;
		std::string m_placeHolderText;
		bool m_isTextEntering = false;
		bool m_wasActive = false;
	public:
		TextInput(const std::string& name = "")
			: AbstractInput(name)
		{}

		void setPlaceHolderText(const std::string& text)
		{
			m_placeHolderText = text;
		}

		void setInputText(const std::string& inputText) {
			m_inputValue = inputText;
		}

		const std::string& getInputText() {
			return m_inputValue;
		}

		bool isTextEntering() {
			return m_isTextEntering;
		}

	protected:
		void renderInput() override {
			pushWidthParam();
			pushFontParam();

			renderTextInput();

			popFontParam();
			popWidthParam();
		}

		virtual void renderTextInput() {
			auto pText = &m_inputValue;
			if (!m_placeHolderText.empty() && !m_wasActive)
				pText = &m_placeHolderText;
			m_isTextEntering = ImGui::InputText(getName().c_str(), pText, getFlags());
			processGenericEvents();
			m_wasActive = isActive() || isHovered();
		}
	};

	class BoolInput
		: public AbstractInput
	{
		bool m_isClicked = false;
		bool m_value = false;
	public:
		BoolInput(const std::string& name = "##", bool value = false)
			: AbstractInput(name), m_value(value)
		{}

		bool present()
		{
			show();
			return isClicked();
		}

		bool isClicked() {
			return m_isClicked;
		}

		void setInputValue(bool value) {
			m_value = value;
		}

		bool isSelected() {
			return m_value;
		}

	protected:
		void renderInput() override {
			m_isClicked = ImGui::Checkbox(getName().c_str(), &m_value);
			processGenericEvents();
		}
	};

	class IntegerInput
		: public AbstractInput,
		public Attribute::Size
	{
		int m_value = 0;
		int m_step = 1;
		int m_fastStep = 100;
		bool m_isValueEntering = false;
	public:
		IntegerInput(const std::string& name = "##")
			: AbstractInput(name)
		{}

		void setInputValue(int value) {
			m_value = value;
		}

		int getInputValue() {
			return m_value;
		}

	protected:
		void renderControl() override {
			pushWidthParam();

			m_isValueEntering = ImGui::InputInt(getName().c_str(), &m_value, m_step, m_fastStep, getFlags());
			processGenericEvents();

			popWidthParam();
		}
	};

	class FloatInput
		: public AbstractInput,
		public Attribute::Size
	{
		float m_value = 0;
		float m_step = 0.f;
		float m_fastStep = 0.0;
		bool m_isValueEntering = false;
	public:
		FloatInput(const std::string& name = "##")
			: AbstractInput(name)
		{}

		void setInputValue(float value) {
			m_value = value;
		}

		float getInputValue() {
			return m_value;
		}

	protected:
		void renderControl() override {
			pushWidthParam();

			m_isValueEntering = ImGui::InputFloat(getName().c_str(), &m_value, m_step, m_fastStep, "%.3f", getFlags());
			processGenericEvents();

			popWidthParam();
		}
	};

	class DoubleInput
		: public AbstractInput,
		public Attribute::Size
	{
		double m_value = 0;
		double m_step = 0.0;
		double m_fastStep = 0.0;
		bool m_isValueEntering = false;
	public:
		DoubleInput(const std::string& name = "##")
			: AbstractInput(name)
		{}

		void setInputValue(double value) {
			m_value = value;
		}

		double getInputValue() {
			return m_value;
		}

	protected:
		void renderControl() override {
			pushWidthParam();

			m_isValueEntering = ImGui::InputDouble(getName().c_str(), &m_value, m_step, m_fastStep, "%.6f", getFlags());
			processGenericEvents();

			popWidthParam();
		}
	};
};