#include "Core/Image/ImageSection.h"

using namespace sda;

Offset ImageSection::getMinOffset() const {
    return m_relVirtualAddress;
}

Offset ImageSection::getMaxOffset() const {
    return m_relVirtualAddress + m_virtualSize;
}

bool ImageSection::contains(Offset offset) const {
    return offset >= getMinOffset() && offset < getMaxOffset();
}

Offset ImageSection::toOffset(size_t offset) const {
    return offset - m_pointerToRawData + m_relVirtualAddress;
}

size_t ImageSection::toImageFileOffset(Offset offset) const {
    return offset - m_relVirtualAddress + m_pointerToRawData;
}