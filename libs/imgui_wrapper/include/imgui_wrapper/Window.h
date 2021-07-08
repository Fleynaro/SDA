#pragma once
#include "Events.h"
#include "controls/Control.h"
#include "controls/AbstractPanel.h"

namespace GUI
{
	class StdWindow :
		public Control,
		public Attribute::Id,
		public Attribute::Flags<
		ImGuiWindowFlags,
		ImGuiWindowFlags_None
		>,
		public GenericEvents
	{
	protected:
		AbstractPanel* m_panel;
		
		ImVec2 m_pos;
		ImVec2 m_size;

		bool m_isOpened = true;
		bool m_fullscreen = false;
	public:
		StdWindow(AbstractPanel* panel, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
			: m_panel(panel), Attribute::Flags<ImGuiWindowFlags, ImGuiWindowFlags_None>(flags)
		{}

		~StdWindow() override
		{
			delete m_panel;
		}

		AbstractPanel* getPanel() const
		{
			return m_panel;
		}

		void setPos(ImVec2 pos) {
			m_pos = pos;
		}

		ImVec2& getPos() {
			return m_pos;
		}

		void setSize(ImVec2 size) {
			m_size = size;
		}

		ImVec2& getSize() {
			return m_size;
		}

		bool isOpened()
		{
			return m_isOpened;
		}

		bool isRemoved() override {
			return !m_isOpened;
		}

		virtual void open()
		{
			m_isOpened = true;
		}

		void close()
		{
			m_isOpened = false;
		}

		void setFullscreen(bool toggle) {
			m_fullscreen = toggle;
		}
	protected:
		void renderControl() override {
			pushParams();
			pushIdParam();
			bool isOpen = ImGui::Begin(m_panel->getName().c_str(), &m_isOpened, getFlags());
			//processGenericEvents();
			
			m_pos = ImGui::GetWindowPos();
			m_size = ImGui::GetWindowSize();

			if (isOpen)
			{
				m_panel->show();
				ImGui::End();
			}
			popIdParam();
		}

		void pushParams() {
			if (m_fullscreen) {
				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(m_pos = viewport->WorkPos);
				ImGui::SetNextWindowSize(m_size = viewport->WorkSize);
				addFlags(ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
			}
			else {
				if (isFlags(ImGuiWindowFlags_NoMove)) {
					ImGui::SetNextWindowPos(m_pos);
				}

				if (isFlags(ImGuiWindowFlags_NoResize)) {
					ImGui::SetNextWindowSize(m_size);
				}
			}
		}
	};

	class PopupBuiltinWindow
		: public StdWindow
	{
		bool m_closeByClickOutside;
		bool m_closeByTimer;
		uint64_t m_lastHoveredOutTime = 0;
	public:
		PopupBuiltinWindow(AbstractPanel* panel, bool closeByClickOutside = true, bool closeByTimer = false)
			: StdWindow(panel), m_closeByClickOutside(closeByClickOutside), m_closeByTimer(closeByTimer)
		{
			m_isOpened = false;
			setFlags(ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
		}

		void open() override
		{
			StdWindow::open();
			m_lastHoveredOutTime = 0;
		}

		void placeAfterItem()
		{
			setPos({ ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y });
		}

	private:
		void renderControl() override {
			if (m_isOpened) {
				pushParams();
				if (ImGui::Begin(getId().c_str(), nullptr, getFlags()))
				{
					processGenericEvents();
					if (m_closeByClickOutside)
					{
						if (!isHovered()) {
							if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) {
								close();
							}
						}
					}
					if(m_closeByTimer)
					{
						if ((!m_lastHoveredOutTime && !isHovered()) || isHoveredOut()) {
							m_lastHoveredOutTime = GetTimeInMs();
						}
						if (!isHovered()) {
							if (GetTimeInMs() - m_lastHoveredOutTime > 2000)
								close();
						}
					}
					
					m_panel->show();
					ImGui::End();
				}
			}
		}

		bool isImGuiHovered() override
		{
			return ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
		}
	};

	class PopupContextWindow
		: public StdWindow
	{
		bool m_isOpenPopup = false;
	public:
		PopupContextWindow(AbstractPanel* panel)
			: StdWindow(panel)
		{
			m_isOpened = false;
		}
		
		void open() override
		{
			StdWindow::open();
			m_isOpenPopup = true;
		}
	
	private:
		void renderControl() override {
			if (m_isOpened) {
				if (m_isOpenPopup) {
					ImGui::OpenPopup(getId().c_str());
					m_isOpenPopup = false;
				}
				ImGui::SetNextWindowFocus();
				if (ImGui::BeginPopupContextItem(getId().c_str()))
				{
					m_panel->show();
					ImGui::EndPopup();
				}
			}
		}
	};

	class PopupModalWindow
		: public StdWindow
	{
		bool m_isOpenPopup = false;
	public:
		PopupModalWindow(AbstractPanel* panel)
			: StdWindow(panel)
		{}

		void open() override
		{
			StdWindow::open();
			m_isOpenPopup = true;
		}

	private:
		void renderControl() override {
			if (m_isOpenPopup) {
				ImGui::OpenPopup(m_panel->getName().c_str());
				m_isOpenPopup = false;
			}
			pushParams();
			bool isOpen = ImGui::BeginPopupModal(m_panel->getName().c_str(), &m_isOpened, getFlags());
			
			m_pos = ImGui::GetWindowPos();
			m_size = ImGui::GetWindowSize();
			
			if (isOpen)
			{
				m_panel->show();
				ImGui::EndPopup();
			}
		}
	};
};