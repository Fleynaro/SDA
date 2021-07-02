#include "SimpleBufferImage.h"

using namespace CE;

CE::SimpleBufferImage::SimpleBufferImage(int8_t* data, int size)
	: m_data(data), m_size(size)
{}

int8_t* CE::SimpleBufferImage::getData() {
	return m_data;
}

int CE::SimpleBufferImage::getSize() {
	return m_size;
}

int CE::SimpleBufferImage::getOffsetOfEntryPoint() {
	return 0;
}

SimpleBufferImage::SegmentType CE::SimpleBufferImage::defineSegment(int offset) {
	return CODE_SEGMENT;
}
