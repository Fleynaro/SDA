#pragma once
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

		bool isClosed() {
			auto isOpened = m_isOpened;
			m_isOpened = true;
			return !isOpened;
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

	private:
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
};