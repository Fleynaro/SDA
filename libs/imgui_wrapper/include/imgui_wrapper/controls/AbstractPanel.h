#pragma once
#include "Control.h"

namespace GUI
{
	class AbstractPanel :
		public Control,
		public Attribute::Name
	{
	public:
		AbstractPanel(const std::string& name = "")
			: Attribute::Name(name)
		{}

	protected:
		virtual void renderPanel() = 0;

		virtual void renderMenuBar() {}
	private:
		void renderControl() override
		{
			if (ImGui::BeginMenuBar()) {
				renderMenuBar();
				ImGui::EndMenuBar();
			}
			renderPanel();
		}
	};

	class StdPanel :
		public AbstractPanel
	{
		EventHandler<> m_drawCall;
	public:
		StdPanel(const std::string& name = "")
			: AbstractPanel(name)
		{}

		StdPanel(std::function<void()> drawCall, const std::string& name = "")
			: AbstractPanel(name), m_drawCall(drawCall)
		{}

		void handler(std::function<void()> drawCall)
		{
			m_drawCall = drawCall;
		}

	private:
		void renderPanel() override
		{
			m_drawCall();
		}
	};
};