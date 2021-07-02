#pragma once
#include "IImage.h"
#include <vector>

namespace CE
{
	class VectorBufferImage : public IImage
	{
		std::vector<int8_t> m_content;
	public:
		VectorBufferImage(std::vector<int8_t> content);

		int8_t* getData() override;

		int getSize() override;

		int getOffsetOfEntryPoint() override;

		SegmentType defineSegment(int offset) override;
	};
};