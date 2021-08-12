#include "SimpleImage.h"

CE::SimpleImage::SimpleImage(IReader* reader)
	: AbstractImage(reader)
{
	ImageSection section;
	section.m_type = ImageSection::CODE_SEGMENT;
	section.m_virtualSize = getSize();
	m_imageSections.push_back(section);

	static int counter = 0;
	counter++;
	m_address = 0x1000000 * counter;
}

int CE::SimpleImage::getOffsetOfEntryPoint() {
	return 0;
}

std::uintptr_t CE::SimpleImage::getAddress() {
	return m_address;
}
