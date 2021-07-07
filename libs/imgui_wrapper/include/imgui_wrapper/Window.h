#pragma once
#include "Events.h"
#include "imgui_internal.h"
#include "controls/Control.h"

namespace GUI
{
	class Window :
		public Control,
		public Attribute::Id,
		public Attribute::Name,
		public Attribute::Flags<
		ImGuiWindowFlags,
		ImGuiWindowFlags_None
		>
	{
	protected:
		bool m_isOpened = true;
		bool m_isFocused = false;

		ImVec2 m_pos;
		ImVec2 m_size;

		bool m_fullscreen = false;
	public:
		Window(const std::string& name, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
			: Attribute::Name(name), Attribute::Flags<ImGuiWindowFlags, ImGuiWindowFlags_None>(flags)
		{}

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

		bool isFocused() {
			return m_isFocused;
		}

		bool isRemoved() override {
			return !m_isOpened;
		}

		void setFullscreen(bool toggle) {
			m_fullscreen = toggle;
		}
	protected:
		void renderControl() override {
			pushParams();
			pushIdParam();
			bool isOpen = ImGui::Begin(getName().c_str(), &m_isOpened, getFlags());

			m_pos = ImGui::GetWindowPos();
			m_size = ImGui::GetWindowSize();

			if (isOpen)
			{
				renderWindow();
				ImGui::End();
			}
			popIdParam();
		}

		virtual void renderWindow() = 0;

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

	/*class AbstractPopupContextWindow
		: public Window,
		public Attribute::Id
	{
	public:
		bool m_hideByClick = false;
		
	protected:
		virtual void renderMenu() = 0;

	private:
		void renderControl() override {
			bool isOpen = true;
			ImGui::SetNextWindowPos({ ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y });
			ImGui::SetNextWindowSize(getSize());
			if (ImGui::Begin(getId().c_str(), &isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
			{
				if (m_hideByClick) {
					if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_RectOnly | ImGuiHoveredFlags_ChildWindows)) {
						if (GetTickCount64() - m_active > 500) {
							if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) {
								setInvisible();
							}
						}
					}
				}
				else {
					sendHoveredEvent();
				}

				renderMenu();
				ImGui::End();
			}
		}
	};*/

	class AbstractPopupContextWindow
		: public Control,
		public Attribute::Id
	{
		bool m_isOpenPopup = false;
	public:
		void open()
		{
			m_isOpenPopup = true;
		}
	
	protected:
		virtual void renderWindow() = 0;
	
	private:
		void renderControl() override {
			if (m_isOpenPopup) {
				ImGui::OpenPopup(getId().c_str());
				m_isOpenPopup = false;
			}
			if (ImGui::BeginPopupContextItem(getId().c_str()))
			{
				renderWindow();
				ImGui::EndPopup();
			}
		}
	};

	class PopupContextWindow
		: public AbstractPopupContextWindow
	{
		EventHandler<> m_drawCall;
	public:
		using AbstractPopupContextWindow::AbstractPopupContextWindow;

		void handler(std::function<void()> drawCall)
		{
			m_drawCall = drawCall;
		}
	
	private:
		void renderWindow() override
		{
			m_drawCall();
		}
	};

	class AbstractPopupModalWindow
		: public Window
	{
		bool m_isOpenPopup = false;
	public:
		using Window::Window;

		void open()
		{
			m_isOpenPopup = true;
		}

	private:
		void renderControl() override {
			if (m_isOpenPopup) {
				ImGui::OpenPopup(getName().c_str());
				m_isOpenPopup = false;
			}
			pushParams();
			bool isOpen = ImGui::BeginPopupModal(getName().c_str(), &m_isOpened, getFlags());
			
			m_pos = ImGui::GetWindowPos();
			m_size = ImGui::GetWindowSize();
			
			if (isOpen)
			{
				renderWindow();
				ImGui::EndPopup();
			}
		}
	};

	class PopupModalWindow
		: public AbstractPopupModalWindow
	{
		EventHandler<> m_drawCall;
	public:
		using AbstractPopupModalWindow::AbstractPopupModalWindow;

		void handler(std::function<void()> drawCall)
		{
			m_drawCall = drawCall;
		}

	private:
		void renderWindow() override
		{
			m_drawCall();
		}
	};
};