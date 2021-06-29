#pragma once
#include "main.h"

namespace CE::Decompiler
{
	// 64-bit mask (max length for all kind of operations). It can be continious (0b0011100) only
	class BitMask64 {
		uint64_t m_bitMask = 0x0;
	public:
		BitMask64() = default;

		BitMask64(uint64_t bitMask)
			: m_bitMask(bitMask)
		{}

		BitMask64(int size, int offset = 0x0)
			: BitMask64(GetBitMask64BySize(size) << uint64_t((offset % 8) * 8))
		{}

		bool isZero() {
			return m_bitMask == 0x0;
		}

		// get mask as integer value
		uint64_t getValue() const {
			return m_bitMask;
		}

		BitMask64 withoutOffset() {
			return m_bitMask >> getOffset();
		}

		// calculate count of 1-bits
		int getBitsCount() const {
			int bitCount = 0;
			for (auto m = m_bitMask; m != 0; m = m >> 1) {
				if (m & 0b1) {
					bitCount++;
				}
			}
			return bitCount;
		}

		int getMaxSizeInBits() const {
			return sizeof(m_bitMask) * 0x8;
		}

		// calculate offset from begin to mask
		int getOffset() const {
			int offset = 0;
			auto mask = m_bitMask;
			auto maxSizeInBits = getMaxSizeInBits() - 1;
			while (offset <= maxSizeInBits && bool(mask & 0b1) == 0) {
				offset += 1;
				mask = mask >> 1;
			}
			return offset;
		}

		// calculate offset from the end
		int getOffsetFromTheEnd() const {
			int offset = 0;
			auto mask = m_bitMask;
			auto maxSizeInBits = getMaxSizeInBits() - 1; // usually it is 63
			while (offset <= maxSizeInBits && bool((mask >> maxSizeInBits) & 0b1) == 0) {
				offset += 1;
				mask = mask << 1;
			}
			return offset;
		}

		// get size of continious mask (in bytes)
		int getSize() const {
			auto bitsCount = getBitsCount();
			return (bitsCount / 8) + ((bitsCount % 8) ? 1 : 0);
		}

		BitMask64 operator&(const BitMask64& b) const {
			return m_bitMask & b.m_bitMask;
		}

		BitMask64 operator|(const BitMask64& b) const {
			return m_bitMask | b.m_bitMask;
		}

		BitMask64 operator~() const {
			return ~m_bitMask;
		}

		BitMask64 operator>>(int offset) const {
			auto val = m_bitMask >> offset;
			return val;
		}

		BitMask64 operator<<(int offset) const {
			return m_bitMask << offset;
		}

		bool operator==(const BitMask64& b) const {
			return m_bitMask == b.m_bitMask;
		}

		bool operator!=(const BitMask64& b) const {
			return !(*this == b);
		}

		bool operator<(const BitMask64& b) const {
			return m_bitMask < b.m_bitMask;
		}

		bool operator<=(const BitMask64& b) const {
			return m_bitMask <= b.m_bitMask;
		}

		static uint64_t GetBitMask64BySizeInBits(int sizeInBits) {
			if (sizeInBits >= 64) // todo: increase from 8 to 16 bytes (it requires 128-bit arithmetic implementation)
				return -1;
			return ((uint64_t)1 << sizeInBits) - 1;
		}
		
		static uint64_t GetBitMask64BySize(int size) {
			return GetBitMask64BySizeInBits(size * 8);
		}
	};

	// non-continious version of bitmask
	class ExprBitMask : public BitMask64
	{
	public:
		ExprBitMask(const BitMask64& bitMask)
			: BitMask64(bitMask)
		{}

		using BitMask64::BitMask64;

		int getSizeInBits() const {
			return getMaxSizeInBits() - getOffsetFromTheEnd();
		}

		// calculate min count of bytes (1, 2, 4, 8) that covers the mask
		int getSize() const {
			auto sizeInBits = getSizeInBits();
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
	};
};