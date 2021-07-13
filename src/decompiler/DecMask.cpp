#include "DecMask.h"

using namespace CE;
using namespace Decompiler;

int ExprBitMask::getSizeInBits() const {
	return getMaxSizeInBits() - getOffsetFromTheEnd();
}

int ExprBitMask::getSize() const {
	const auto sizeInBits = getSizeInBits();
	if (sizeInBits <= 16) {
		if (sizeInBits <= 8)
			return 1;
		return 2;
	}
	else {
		if (sizeInBits <= 32)
			return 4;
		return 8;
	}
}

bool BitMask64::isZero() const
{
	return m_bitMask == 0x0;
}

uint64_t BitMask64::getValue() const {
	return m_bitMask;
}

BitMask64 BitMask64::withoutOffset() const
{
	return m_bitMask >> getOffset();
}

int BitMask64::getBitsCount() const {
	int bitCount = 0;
	for (auto m = m_bitMask; m != 0; m = m >> 1) {
		if (m & 0b1) {
			bitCount++;
		}
	}
	return bitCount;
}

int BitMask64::getMaxSizeInBits() const {
	return sizeof(m_bitMask) * 0x8;
}

int BitMask64::getOffset() const {
	int offset = 0;
	auto mask = m_bitMask;
	const auto maxSizeInBits = getMaxSizeInBits() - 1;
	while (offset <= maxSizeInBits && static_cast<bool>(mask & 0b1) == 0) {
		offset += 1;
		mask = mask >> 1;
	}
	return offset;
}

int BitMask64::getOffsetFromTheEnd() const {
	int offset = 0;
	auto mask = m_bitMask;
	const auto maxSizeInBits = getMaxSizeInBits() - 1; // usually it is 63
	while (offset <= maxSizeInBits && static_cast<bool>((mask >> maxSizeInBits) & 0b1) == 0) {
		offset += 1;
		mask = mask << 1;
	}
	return offset;
}

int BitMask64::getSize() const {
	const auto bitsCount = getBitsCount();
	return (bitsCount / 8) + ((bitsCount % 8) ? 1 : 0);
}

uint64_t BitMask64::GetBitMask64BySizeInBits(int sizeInBits) {
	if (sizeInBits >= 64) // todo: increase from 8 to 16 bytes (it requires 128-bit arithmetic implementation)
		return -1;
	return (static_cast<uint64_t>(1) << sizeInBits) - 1;
}

uint64_t BitMask64::GetBitMask64BySize(int size) {
	return GetBitMask64BySizeInBits(size * 8);
}
