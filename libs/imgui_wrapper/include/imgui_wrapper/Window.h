#pragma once
#include "Events.h"
#include "controls/Control.h"
#include "controls/AbstractPanel.h"

namespace GUI
{
	static ImVec2 GetLeftBottom() {
		return { ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y };
	}
	
	class StdWindow :
		public Control,
		public Attribute::Pos,
		public Attribute::Size,
		public Attribute::Flags<
		ImGuiWindowFlags,
		ImGuiWindowFlags_None
		>,
		public WindowsGenericEvents
	{
	protected:
		AbstractPanel* m_panel;

		bool m_isOpened = true;
		bool m_fullscreen = false;
		ImGuiID m_dockspaceId = 0;
	public:
		bool m_applyPosAndSize = false;
		
		StdWindow(AbstractPanel* panel, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
			: m_panel(panel), Flags<ImGuiWindowFlags, ImGuiWindowFlags_None>(flags)
		{
			m_panel->m_window = this;
		}

		~StdWindow() override
		{
			delete m_panel;
		}

		AbstractPanel* getPanel() const
		{
			return m_panel;
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

		void setDockSpace(ImGuiID dockspaceId) {
			m_dockspaceId = dockspaceId;
		}
	
	protected:
		void renderControl() override {
			pushParams();
			if(m_dockspaceId)
				ImGui::SetNextWindowDockID(m_dockspaceId, ImGuiCond_FirstUseEver);
			bool isOpen = ImGui::Begin(m_panel->getName().c_str(), &m_isOpened, getFlags());
			processGenericEvents();
			
			m_pos = ImGui::GetWindowPos();
			m_size = ImGui::GetWindowSize();

			if (isOpen)
			{
				m_panel->show();
			}
			ImGui::End();
		}

		void pushParams() {
			if (m_fullscreen) {
				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(m_pos = viewport->WorkPos);
				ImGui::SetNextWindowSize(m_size = viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
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

			if(m_applyPosAndSize) {
				ImGui::SetNextWindowPos(m_pos);
				ImGui::SetNextWindowSize(m_size);
				m_applyPosAndSize = false;
			}
		}

		bool isImGuiHovered() override {
			return ImGui::IsWindowHovered();
		}

		bool isImGuiFocused() override {
			return ImGui::IsWindowFocused();
		}
	};

	class PopupBuiltinWindow
		: public StdWindow,
		public Attribute::Id
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
			getPos() = GetLeftBottom();
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
				}
				ImGui::End();
			}
		}

		bool isImGuiHovered() override
		{
			return ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
		}
	};

	class PopupContextWindow
		: public StdWindow,
		public Attribute::Id
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
					processGenericEvents();
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
			processGenericEvents();
			
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