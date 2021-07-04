#pragma once
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
	};
};