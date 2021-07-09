#include "VectorBufferImage.h"

using namespace CE;

CE::VectorBufferImage::VectorBufferImage(std::vector<int8_t> content)
	: m_content(content)
{
	ImageSection section;
	section.m_type = ImageSection::CODE_SEGMENT;
	section.m_virtualSize = (int)content.size();
	m_imageSections.push_back(section);
}

int8_t* CE::VectorBufferImage::getData() {
	return m_content.data();
}

int CE::VectorBufferImage::getSize() {
	return (int)m_content.size();
}

int CE::VectorBufferImage::getOffsetOfEntryPoint() {
	return 0;
}