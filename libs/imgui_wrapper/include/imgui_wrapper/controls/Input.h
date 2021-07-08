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
			renderInput();
			drawInputBorder();
			popIdParam();
		}

		virtual void renderInput() = 0;
	};

	class TextInput
		: public AbstractInput,
		public Attribute::Width,
		public Attribute::Font
	{
		std::string m_inputValue;
		bool m_isTextEntering = false;
	public:
		TextInput(const std::string& name = "")
			: AbstractInput(name)
		{}

		void setInputText(const std::string& inputText) {
			m_inputValue = inputText;
		}

		const std::string& getInputText() {
			return m_inputValue;
		}

		bool isTextEntering() {
			return CheckEventFlag(m_isTextEntering);
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
			if (ImGui::InputText(getName().c_str(), &m_inputValue, getFlags())) {
				m_isTextEntering = true;
			}
		}
	};

	class BoolInput
		: public AbstractInput
	{
		bool m_isClicked = false;
		bool m_value = false;
		bool m_tooltip = false;
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
			return CheckEventFlag(m_isClicked);
		}

		void setInputValue(bool value) {
			m_value = value;
		}

		bool getInputValue() {
			return m_value;
		}

		bool isSelected() {
			return m_value;
		}

		void setToolTip(bool toggle) {
			m_tooltip = toggle;
		}

	protected:
		void renderInput() override {
			m_isClicked = ImGui::Checkbox(m_tooltip ? "##tooltip" : getName().c_str(), &m_value);
			if (m_tooltip && ImGui::IsItemHovered())
				ImGui::SetTooltip(getName().c_str());
		}
	};

	class IntegerInput
		: public AbstractInput,
		public Attribute::Width
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

			if (ImGui::InputInt(getName().c_str(), &m_value, m_step, m_fastStep, getFlags())) {
				m_isValueEntering = true;
			}

			popWidthParam();
		}
	};

	class FloatInput
		: public AbstractInput,
		public Attribute::Width
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

			if (ImGui::InputFloat(getName().c_str(), &m_value, m_step, m_fastStep, "%.3f", getFlags())) {
				m_isValueEntering = true;
			}

			popWidthParam();
		}
	};

	class DoubleInput
		: public AbstractInput,
		public Attribute::Width
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

			if (ImGui::InputDouble(getName().c_str(), &m_value, m_step, m_fastStep, "%.6f", getFlags())) {
				m_isValueEntering = true;
			}

			popWidthParam();
		}
	};
};