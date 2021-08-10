#include "AbstractImage.h"

CE::AbstractImage::AbstractImage(IReader* reader): m_reader(reader) {
}

CE::AbstractImage::~AbstractImage() {
	delete m_reader;
}

CE::IReader* CE::AbstractImage::getReader() const {
	return m_reader;
}

void CE::AbstractImage::read(Offset offset, std::vector<uint8_t>& data) const {
	m_reader->read(toImageOffset(offset), data);
}

int CE::AbstractImage::getSize() const {
	return m_reader->getSize();
}

std::uintptr_t CE::AbstractImage::getAddress() {
	return 0x0;
}

const std::list<CE::ImageSection>& CE::AbstractImage::getImageSections() const {
	return m_imageSections;
}

uint64_t CE::AbstractImage::addrToRva(uint64_t addr) {
	return addr - getAddress();
}

uint64_t CE::AbstractImage::toImageOffset(Offset offset) const {
	const auto section = getSectionByOffset(offset);
	if (!section)
		return offset;
	return section->toImageOffset(offset);
}

const CE::ImageSection* CE::AbstractImage::getSectionByOffset(Offset offset) const {
	for (const auto& section : getImageSections()) {
		if (section.contains(offset)) {
			return &section;
		}
	}
	return &DefaultSection;
}
