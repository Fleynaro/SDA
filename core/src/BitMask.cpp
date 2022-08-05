#include "Core/BitMask.h"

using namespace sda;

size_t GetBitMask64BySizeInBits(size_t sizeInBits) {
	if (sizeInBits >= MaxMaskSizeInBits) // todo: increase from 8 to 16 bytes (it requires 128-bit arithmetic implementation)
		return -1;
	return (static_cast<size_t>(1) << sizeInBits) - 1;
}

size_t GetBitMask64BySize(size_t size) {
	return GetBitMask64BySizeInBits(size * BitsInBytes);
}

BitMask::BitMask(Value mask)
    : m_mask(mask)
{}

BitMask::BitMask(size_t size, size_t offset)
    : m_mask(GetBitMask64BySize(size) << (offset % MaxMaskSizeInBytes) * BitsInBytes)
{}

size_t BitMask::getMaxSizeInBits() const {
	return sizeof(Value) * BitsInBytes;
}

size_t BitMask::getSize() const {
	size_t bitsCount = 0;
	for (auto m = m_mask; m != 0; m = m >> 1) {
		if (m & 0b1) {
			bitsCount++;
		}
	}
	return (bitsCount / 8) + ((bitsCount % 8) ? 1 : 0);
}

size_t BitMask::getOffset() const {
	size_t offset = 0;
	auto mask = m_mask;
	const auto maxSizeInBits = getMaxSizeInBits() - 1;
	while (offset <= maxSizeInBits && static_cast<bool>(mask & 0b1) == 0) {
		offset += 1;
		mask = mask >> 1;
	}
	return offset;
}

BitMask::operator size_t() const {
    return m_mask;
}