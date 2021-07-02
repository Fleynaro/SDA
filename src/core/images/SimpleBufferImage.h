#pragma once
#include "IImage.h"

namespace CE
{
	class SimpleBufferImage : public IImage
	{
		int8_t* m_data;
		int m_size;
	public:
		SimpleBufferImage(int8_t* data, int size);

		int8_t* getData() override;

		int getSize() override;

		int getOffsetOfEntryPoint() override;

		SegmentType defineSegment(int offset) override;
	};
};