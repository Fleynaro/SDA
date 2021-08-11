#pragma once
#include <chrono>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
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
	protected:
		virtual ~Control() {};

		virtual void renderControl() = 0;

	public:
		void show() {
			renderControl();
		}

		virtual bool isRemoved() {
			return false;
		}

		template<typename T = Control>
		static void Show(T*& control) {
			if (control) {
				control->show();
				if (control->isRemoved()) {
					delete control;
					control = nullptr;
				}
			}
		}
	};

	static uint64_t GetTimeInMs() {
		auto time_since_epoch = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count();
	}
};