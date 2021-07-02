#include "VectorBufferImage.h"

using namespace CE;

CE::VectorBufferImage::VectorBufferImage(std::vector<int8_t> content)
	: m_content(content)
{}

int8_t* CE::VectorBufferImage::getData() {
	return m_content.data();
}

int CE::VectorBufferImage::getSize() {
	return (int)m_content.size();
}

int CE::VectorBufferImage::getOffsetOfEntryPoint() {
	return 0;
}

VectorBufferImage::SegmentType CE::VectorBufferImage::defineSegment(int offset) {
	if (offset >= 0 && offset < m_content.size())
		return CODE_SEGMENT;
	return NONE_SEGMENT;
}
