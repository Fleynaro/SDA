#pragma once
#include "IImage.h"

namespace CE
{
	class SimpleBufferImage : public IImage
	{
		int8_t* m_data;
		int m_size;
	public:
		SimpleBufferImage(int8_t* data, int size)
			: m_data(data), m_size(size)
		{}

		int8_t* getData() override {
			return m_data;
		}

		int getSize() override {
			return m_size;
		}

		int getOffsetOfEntryPoint() override {
			return 0;
		}

		SegmentType defineSegment(int offset) override {
			return CODE_SEGMENT;
		}
	};
};