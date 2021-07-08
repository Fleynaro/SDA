#pragma once
#include <chrono>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "../Attribute.h"
#include <string>


namespace GUI
{
	using Color = uint32_t;
	using ColorRGBA = Color;
	using ColorComponent = int;
	
	class Control
	{
		bool m_display = true;
	protected:
		virtual ~Control() {};

		virtual void renderControl() = 0;

	public:
		void show() {
			if (isShown()) {
				renderControl();
			}
		}

		void setDisplay(bool toggle) {
			m_display = toggle;
		}

		virtual bool isShown() {
			return m_display;
		}

		virtual bool isRemoved()
		{
			return false;
		}

		template<typename T = Control>
		static void Show(T*& control)
		{
			if (control)
			{
				control->show();
				if (control->isRemoved()) {
					delete control;
					control = nullptr;
				}
			}
		}
	};

	static uint64_t GetTimeInMs()
	{
		auto time_since_epoch = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count();
	}
};