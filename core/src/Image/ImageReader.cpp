#include "Core/Image/ImageReader.h"

using namespace sda;

PointerImageReader::PointerImageReader(uint8_t* data, size_t size)
    : m_data(data), m_size(size)
{}

void PointerImageReader::readBytesAtOffset(uint64_t offset, std::vector<uint8_t>& data) {
    const auto size = std::min(getImageSize() - offset, data.size());
	std::copy_n(&m_data[offset], size, data.begin());
}

size_t PointerImageReader::getImageSize() {
    return m_size;
}

VectorImageReader::VectorImageReader(const std::vector<uint8_t>& data)
    : m_data(data)
{}

void VectorImageReader::readBytesAtOffset(uint64_t offset, std::vector<uint8_t>& data) {
    const auto size = std::min(getImageSize() - offset, data.size());
    std::copy_n(m_data.begin() + offset, size, data.begin());
}

size_t VectorImageReader::getImageSize() {
    return m_data.size();
}