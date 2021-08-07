#pragma once
#include "Events.h"
#include "controls/Control.h"
#include "controls/AbstractPanel.h"
#include "controls/Text.h"
#include "controls/Button.h"
#include <stdexcept>

namespace GUI
{
	class PopupModalWindow;

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
		bool m_focus = false;
	public:
		bool m_applyPosAndSize = false;
		
		StdWindow(AbstractPanel* panel, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
			: m_panel(panel), Flags<ImGuiWindowFlags, ImGuiWindowFlags_None>(flags)
		{
			m_panel->m_window = this;
		}

		~StdWindow() override {
			delete m_panel;
		}

		AbstractPanel* getPanel() const {
			return m_panel;
		}

		bool isOpened() {
			return m_isOpened;
		}

		bool isRemoved() override {
			return !m_isOpened;
		}

		virtual void open() {
			m_isOpened = true;
		}

		void close() {
			m_isOpened = false;
		}

		void setFullscreen(bool toggle) {
			m_fullscreen = toggle;
		}

		void focus() {
			m_focus = true;
		}

	protected:
		void renderControl() override {
			pushParams();
			if(m_focus) {
				ImGui::SetNextWindowFocus();
				m_focus = false;
			}
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

				if (isFlags(ImGuiWindowFlags_NoResize) && !isFlags(ImGuiWindowFlags_AlwaysAutoResize)) {
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
		uint64_t m_lastHoveredOutTime = 0;
		bool m_isOpenPopup = false;
	public:
		int m_closeTimerMs = 0;
		
		PopupBuiltinWindow(AbstractPanel* panel)
			: StdWindow(panel)
		{
			m_isOpened = false;
			setFlags(ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
		}

		void open() override
		{
			StdWindow::open();
			m_lastHoveredOutTime = 0;
			m_isOpenPopup = true;
		}

		void placeAfterItem()
		{
			getPos() = GetLeftBottom();
		}

	private:
		void renderControl() override {
			if (m_isOpenPopup) {
				ImGui::OpenPopup(getId().c_str());
				m_isOpenPopup = false;
			}
			
			pushParams();
			ImGuiContext& g = *GImGui;
			ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
			if ((m_isOpened = ImGui::BeginPopupEx(g.CurrentWindow->GetID(getId().c_str()), getFlags())))
			{
				processGenericEvents();
				if(m_closeTimerMs)
				{
					if ((!m_lastHoveredOutTime && !isHovered()) || isHoveredOut()) {
						m_lastHoveredOutTime = GetTimeInMs();
					}
					if (!isHovered()) {
						if (GetTimeInMs() - m_lastHoveredOutTime > m_closeTimerMs)
							close();
					}
				}
				
				m_panel->show();
				ImGui::EndPopup();
			}
			ImGui::PopItemFlag();
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
			if (m_panel->getName().empty())
				throw std::logic_error("empty name");
			
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

	class WindowManager : public Control
	{
		std::vector<StdWindow*> m_windows;
	public:
		ImGuiID m_dockSpaceId = 0;
		
		WindowManager() {}

		~WindowManager() override {
			for (const auto window : m_windows)
				delete window;
		}

		const std::vector<StdWindow*>& getWindows() const {
			return m_windows;
		}

		template<typename T = AbstractPanel>
		StdWindow* findWindow(const std::function<bool(T*)>& filter) const {
			for (const auto window : m_windows) {
				if (window && filter(dynamic_cast<T*>(window->getPanel())))
					return window;
			}
			return nullptr;
		}
		
		void addWindow(StdWindow* window) {
			for(int i = 0; i < m_windows.size(); i ++) {
				if(!m_windows[i]) {
					m_windows[i] = window;
					AddIdSuffix(window, i);
					return;
				}
			}
			AddIdSuffix(window, static_cast<int>(m_windows.size()));
			m_windows.push_back(window);
		}

	protected:
		void renderControl() override {
			for (int i = 0; i < m_windows.size(); i++) {
				if (!m_windows[i])
					continue;
				if (m_dockSpaceId)
					ImGui::SetNextWindowDockID(m_dockSpaceId, ImGuiCond_FirstUseEver);
				m_windows[i]->show();
				if (m_windows[i]->isRemoved()) {
					delete m_windows[i];
					m_windows[i] = nullptr;
				}
			}
		}

		static void AddIdSuffix(StdWindow* window, int id) {
			window->getPanel()->setName(window->getPanel()->getName() + std::to_string(id));
		}
	};

	static PopupModalWindow* CreateMessageWindow(const std::string& message) {
		const auto panel = new StdPanel("Message");
		panel->handler([message, panel]()
			{
				Text::Text::Text(message).show();
				NewLine();
				if (Button::StdButton("Ok").present()) {
					panel->m_window->close();
				}
			});
		const auto messageWindow = new PopupModalWindow(panel);
		messageWindow->open();
		return messageWindow;
	}
};