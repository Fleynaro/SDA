#include "VectorBufferImage.h"

using namespace CE;

VectorBufferImage::VectorBufferImage(std::vector<int8_t> content)
	: m_content(content)
{
	ImageSection section;
	section.m_type = ImageSection::CODE_SEGMENT;
	section.m_virtualSize = static_cast<int>(content.size());
	m_imageSections.push_back(section);
}

int8_t* VectorBufferImage::getData() {
	return m_content.data();
}

int VectorBufferImage::getSize() {
	return static_cast<int>(m_content.size());
}

int VectorBufferImage::getOffsetOfEntryPoint() {
	return 0;
}