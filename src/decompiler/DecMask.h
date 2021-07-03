#pragma once
#include <stdint.h>

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

		bool isZero() const;

		// get mask as integer value
		uint64_t getValue() const;

		BitMask64 withoutOffset() const;

		// calculate count of 1-bits
		int getBitsCount() const;

		int getMaxSizeInBits() const;

		// calculate offset from begin to mask
		int getOffset() const;

		// calculate offset from the end
		int getOffsetFromTheEnd() const;

		// get size of continious mask (in bytes)
		int getSize() const;

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

		static uint64_t GetBitMask64BySizeInBits(int sizeInBits);
		
		static uint64_t GetBitMask64BySize(int size);
	};

	// non-continious version of bitmask
	class ExprBitMask : public BitMask64
	{
	public:
		ExprBitMask(const BitMask64& bitMask)
			: BitMask64(bitMask)
		{}

		using BitMask64::BitMask64;

		int getSizeInBits() const;

		// calculate min count of bytes (1, 2, 4, 8) that covers the mask
		int getSize() const;
	};
};