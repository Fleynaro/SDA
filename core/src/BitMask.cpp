#include "Core/BitMask.h"

using namespace sda;

size_t GetBitMask64BySizeInBits(size_t sizeInBits) {
	if (sizeInBits >= 64) // todo: increase from 8 to 16 bytes (it requires 128-bit arithmetic implementation)
		return -1;
	return (static_cast<size_t>(1) << sizeInBits) - 1;
}

size_t GetBitMask64BySize(size_t size) {
	return GetBitMask64BySizeInBits(size * 8);
}

BitMask::BitMask(Value mask)
    : m_mask(mask)
{}

BitMask::BitMask(size_t size, size_t offset)
    : m_mask(GetBitMask64BySize(size) << (offset % 8) * 8)
{}

BitMask::operator size_t() const {
    return m_mask;
}