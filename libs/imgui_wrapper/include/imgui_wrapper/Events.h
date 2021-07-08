#pragma once
#include <functional>
#include <imgui.h>

namespace GUI
{
	static bool CheckEventFlag(bool& value) {
		auto result = value;
		value = false;
		return result;
	}
	
	class GenericEvents
	{
		bool m_isClickedByLeftMouseBtn = false;
		bool m_isClickedByRightMouseBtn = false;
		bool m_isClickedByMiddleMouseBtn = false;

		bool m_isHovered = false;
		bool m_isHoveredIn = false;
		bool m_isHoveredOut = false;

		bool m_isFocused = false;
		bool m_isFocusedIn = false;
		bool m_isFocusedOut = false;

		bool m_isVisible = false;
		bool m_isVisibleOn = false;
		bool m_isVisibleOff = false;
	public:
		GenericEvents(bool processEvents = false)
		{
			if(processEvents)
				processGenericEvents();
		}
		
		bool isClickedByLeftMouseBtn() {
			return CheckEventFlag(m_isClickedByLeftMouseBtn);
		}

		bool isClickedByRightMouseBtn() {
			return CheckEventFlag(m_isClickedByRightMouseBtn);
		}

		bool isClickedByMiddleMouseBtn() {
			return CheckEventFlag(m_isClickedByMiddleMouseBtn);
		}

		bool isHovered() const
		{
			return m_isHovered;
		}

		bool isHoveredIn() {
			return CheckEventFlag(m_isHoveredIn);
		}

		bool isHoveredOut() {
			return CheckEventFlag(m_isHoveredOut);
		}

		bool isFocused() const
		{
			return m_isFocused;
		}

		bool isFocusedIn() {
			return CheckEventFlag(m_isFocusedIn);
		}

		bool isFocusedOut() {
			return CheckEventFlag(m_isFocusedOut);
		}

		bool isVisible() const
		{
			return m_isVisible;
		}

		bool isVisibleOn() {
			return CheckEventFlag(m_isVisibleOn);
		}

		bool isVisibleOff() {
			return CheckEventFlag(m_isVisibleOff);
		}

	protected:
		virtual bool isImGuiHovered() {
			return ImGui::IsItemHovered();
		}

		virtual bool isImGuiFocused() {
			return ImGui::IsItemFocused();
		}
		
		virtual bool isImGuiVisible() {
			return ImGui::IsItemVisible();
		}
		
		void processGenericEvents() {
			// mouse
			if (isImGuiHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				m_isClickedByLeftMouseBtn = true;

			if (isImGuiHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				m_isClickedByRightMouseBtn = true;

			if (isImGuiHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
				m_isClickedByMiddleMouseBtn = true;

			// hover
			if (isImGuiHovered()) {
				if (!m_isHovered) {
					m_isHovered = true;
					m_isHoveredIn = true;
					m_isHoveredOut = false;
				}
			}
			else {
				if (m_isHovered) {
					m_isHovered = false;
					m_isHoveredIn = false;
					m_isHoveredOut = true;
				}
			}

			// focus
			if (isImGuiFocused()) {
				if (!m_isFocused) {
					m_isFocused = true;
					m_isFocusedIn = true;
					m_isFocusedOut = false;
				}
			}
			else {
				if (m_isFocused) {
					m_isFocused = false;
					m_isFocusedIn = false;
					m_isFocusedOut = true;
				}
			}

			// visibility
			if (isImGuiVisible()) {
				if (!m_isVisible) {
					m_isVisible = true;
					m_isVisibleOn = true;
					m_isVisibleOff = false;
				}
			}
			else {
				if (m_isVisible) {
					m_isVisible = false;
					m_isVisibleOn = false;
					m_isVisibleOff = true;
				}
			}
		}
	};

	template<typename ...Args>
	class EventHandler
	{
		bool m_isInit = false;
		std::function<void(Args...)> m_function;
	public:
		EventHandler() = default;
		
		EventHandler(const std::function<void(Args...)>& function)
			: m_function(function), m_isInit(true)
		{}

		bool isInit() const
		{
			return m_isInit;
		}

		void operator()(Args... args)
		{
			if(m_isInit)
				m_function(args...);
		}
	};
};