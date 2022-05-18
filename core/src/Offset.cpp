#include "Core/Offset.h"

using namespace sda;

ComplexOffset::ComplexOffset(size_t fullOffset)
    : m_fullOffset(fullOffset)
{}

ComplexOffset::ComplexOffset(Offset byteOffset, size_t index)
    : m_byteOffset(byteOffset), m_index(index)
{}

size_t ComplexOffset::getIndex() const {
    return m_index;
}

Offset ComplexOffset::getByteOffset() const {
    return m_byteOffset;
}

ComplexOffset::operator size_t() const {
    return m_fullOffset;
}