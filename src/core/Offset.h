#pragma once
#include <cstdint>

namespace CE
{
	inline const static uint64_t InvalidOffset = -1;

	// it is a pseudonym of RVA (relative virtual address)
	using Offset = uint64_t;

	// it consists of {byte offset} and {item order id} parts (used for pcode instructions)
	class ComplexOffset
	{
		union {
			struct {
				uint64_t m_orderId : 8;
				uint64_t m_byteOffset : 56;
			};
			struct {
				uint64_t m_fullOffset : 64;
			};
		};

	public:
		ComplexOffset(uint64_t fullOffset = InvalidOffset)
			: m_fullOffset(fullOffset)
		{}
		
		ComplexOffset(Offset byteOffset, int orderId)
			: m_byteOffset(byteOffset), m_orderId(orderId)
		{}

		int getOrderId() const {
			return m_orderId;
		}

		Offset getByteOffset() const {
			return m_byteOffset;
		}

		operator uint64_t() const {
			return *reinterpret_cast<const uint64_t*>(this);
		}
	};
};