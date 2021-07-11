#pragma once
#include "Control.h"

namespace GUI
{
	class Progress
		: public Control,
		public Attribute::Size
	{
		float m_min;
		float m_max;
		float m_value;
		float m_width = 300.f;
		float m_height = 20.f;
	public:
		Progress(float value, float min = 0.f, float max = 100.f)
			: m_value(value), m_min(min), m_max(max)
		{}

		float getFraction() {
			return (m_value - m_min) / (m_max - m_min);
		}

		float getPercent() {
			return getFraction() * 100.f;
		}

		Progress* setValue(float value) {
			m_value = value;
			return this;
		}

		Progress* setMin(float value) {
			m_min = value;
			return this;
		}

		Progress* setMax(float value) {
			m_max = value;
			return this;
		}
	protected:
		void renderControl() override {
			ImGui::ProgressBar(getFraction(), m_size);
		}
	};
};