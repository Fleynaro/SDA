#pragma once
#include "Control.h"
#include "imgui_wrapper/Events.h"

namespace GUI::Button
{
	class AbstractButton
		: public Control,
		public GenericEvents,
		public Attribute::Id,
		public Attribute::Name
	{
	public:
		AbstractButton(const std::string& name)
			: Attribute::Name(name)
		{}

		bool present() {
			show();
			return isClicked();
		}

		bool isClicked() const {
			return m_isClicked;
		}
	protected:
		bool m_isClicked = false;

		void renderControl() override {
			pushIdParam();
			renderButton();
			processGenericEvents();
			popIdParam();
		}

		virtual void renderButton() = 0;
	};

	class StdButton
		: public AbstractButton,
		public Attribute::Size,
		public Attribute::Font
	{
	public:
		StdButton(const std::string& name = "")
			: AbstractButton(name)
		{}

	protected:
		void renderButton() override {
			pushFontParam();

			m_isClicked = ImGui::Button(getName().c_str(), m_size);

			popFontParam();
		}
	};

	class ButtonArrow
		: public AbstractButton
	{
		ImGuiDir m_direction;
	public:
		ButtonArrow(ImGuiDir direction)
			: AbstractButton("##"), m_direction(direction)
		{}

	protected:
		void renderButton() override
		{
			m_isClicked = ImGui::ArrowButton(getName().c_str(), m_direction);
		}
	};

	class ButtonSmall
		: public AbstractButton,
		public Attribute::Font
	{
	public:
		ButtonSmall(const std::string& name)
			: AbstractButton(name)
		{}

	protected:
		void renderButton() override
		{
			pushFontParam();

			m_isClicked = ImGui::SmallButton(getName().c_str());

			popFontParam();
		}
	};
};