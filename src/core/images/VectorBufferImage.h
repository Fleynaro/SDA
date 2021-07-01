#pragma once
#include "IImage.h"
#include <vector>

namespace CE
{
	class VectorBufferImage : public IImage
	{
		std::vector<int8_t> m_content;
	public:
		VectorBufferImage(std::vector<int8_t> content)
			: m_content(content)
		{}

		int8_t* getData() override {
			return m_content.data();
		}

		int getSize() override {
			return (int)m_content.size();
		}

		int getOffsetOfEntryPoint() override {
			return 0;
		}

		SegmentType defineSegment(int offset) override {
			if(offset >= 0 && offset < m_content.size())
				return CODE_SEGMENT;
			return NONE_SEGMENT;
		}
	};
};