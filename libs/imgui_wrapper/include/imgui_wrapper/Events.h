#pragma once
#include <functional>
#include <imgui.h>

namespace GUI
{
	class AbstractGenericEvents
	{
		bool m_isHovered = false;
		bool m_isHoveredIn = false;
		bool m_isHoveredOut = false;

		bool m_isFocused = false;
		bool m_isFocusedIn = false;
		bool m_isFocusedOut = false;

	public:
		bool isHovered() const
		{
			return m_isHovered;
		}

		bool isHoveredIn() const
		{
			return m_isHoveredIn;
		}

		bool isHoveredOut() const
		{
			return m_isHoveredOut;
		}

		bool isFocused() const
		{
			return m_isFocused;
		}

		bool isFocusedIn() const
		{
			return m_isFocusedIn;
		}

		bool isFocusedOut() const
		{
			return m_isFocusedOut;
		}

	protected:
		virtual bool isImGuiHovered() = 0;

		virtual bool isImGuiFocused() = 0;

		virtual void processGenericEvents() {
			m_isHoveredIn = false;
			m_isHoveredOut = false;
			m_isFocusedIn = false;
			m_isFocusedOut = false;
			
			// hover
			if (isImGuiHovered()) {
				if (!m_isHovered) {
					m_isHovered = true;
					m_isHoveredIn = true;
				}
			}
			else {
				if (m_isHovered) {
					m_isHovered = false;
					m_isHoveredOut = true;
				}
			}

			// focus
			if (isImGuiFocused()) {
				if (!m_isFocused) {
					m_isFocused = true;
					m_isFocusedIn = true;
				}
			}
			else {
				if (m_isFocused) {
					m_isFocused = false;
					m_isFocusedOut = true;
				}
			}

		}
	};
	
	class GenericEvents : public AbstractGenericEvents
	{
		bool m_isClickedByLeftMouseBtn = false;
		bool m_isClickedByRightMouseBtn = false;
		bool m_isClickedByMiddleMouseBtn = false;

		bool m_isActive = false;
		bool m_isActiveOn = false;
		bool m_isActiveOff = false;

		bool m_isVisible = false;
		bool m_isVisibleOn = false;
		bool m_isVisibleOff = false;
	public:
		GenericEvents(bool processEvents = false)
		{
			if(processEvents)
				processGenericEvents();
		}
		
		bool isClickedByLeftMouseBtn() const
		{
			return m_isClickedByLeftMouseBtn;
		}

		bool isClickedByRightMouseBtn() const
		{
			return m_isClickedByRightMouseBtn;
		}

		bool isClickedByMiddleMouseBtn() const
		{
			return m_isClickedByMiddleMouseBtn;
		}

		bool isActive() const
		{
			return m_isActive;
		}

		bool isVisible() const
		{
			return m_isVisible;
		}

		bool isVisibleOn() const
		{
			return m_isVisibleOn;
		}

		bool isVisibleOff() const
		{
			return m_isVisibleOff;
		}

	protected:
		virtual bool isImGuiActive() {
			return ImGui::IsItemActive();
		}
		
		bool isImGuiHovered() override {
			return ImGui::IsItemHovered();
		}

		bool isImGuiFocused() override {
			return ImGui::IsItemFocused();
		}
		
		virtual bool isImGuiVisible() {
			return ImGui::IsItemVisible();
		}
		
		void processGenericEvents() {
			AbstractGenericEvents::processGenericEvents();
			m_isClickedByLeftMouseBtn = false;
			m_isClickedByRightMouseBtn = false;
			m_isClickedByMiddleMouseBtn = false;
			m_isActiveOn = false;
			m_isActiveOff = false;
			m_isVisibleOn = false;
			m_isVisibleOff = false;
			
			// mouse
			if (isImGuiHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				m_isClickedByLeftMouseBtn = true;

			if (isImGuiHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				m_isClickedByRightMouseBtn = true;

			if (isImGuiHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
				m_isClickedByMiddleMouseBtn = true;

			// active
			if(isImGuiActive())
			{
				if (!m_isActive) {
					m_isActive = true;
					m_isActiveOn = true;
				}
			} else
			{
				if (m_isActive) {
					m_isActive = false;
					m_isActiveOff = true;
				}
			}

			// visibility
			if (isImGuiVisible()) {
				if (!m_isVisible) {
					m_isVisible = true;
					m_isVisibleOn = true;
				}
			}
			else {
				if (m_isVisible) {
					m_isVisible = false;
					m_isVisibleOff = true;
				}
			}
		}
	};

	class WindowsGenericEvents : public AbstractGenericEvents
	{
	protected:
		bool isImGuiHovered() override {
			return ImGui::IsWindowHovered();
		}

		bool isImGuiFocused() override {
			return ImGui::IsWindowFocused();
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