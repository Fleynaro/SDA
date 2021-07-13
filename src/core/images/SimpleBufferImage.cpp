#include "SimpleBufferImage.h"

using namespace CE;

SimpleBufferImage::SimpleBufferImage(int8_t* data, int size)
	: m_data(data), m_size(size)
{
	ImageSection section;
	section.m_type = ImageSection::CODE_SEGMENT;
	section.m_virtualSize = 0x100000000000;
	m_imageSections.push_back(section);
}

int8_t* SimpleBufferImage::getData() {
	return m_data;
}

int SimpleBufferImage::getSize() {
	return m_size;
}

int SimpleBufferImage::getOffsetOfEntryPoint() {
	return 0;
}