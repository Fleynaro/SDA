#pragma once

namespace sda
{
    inline const static size_t InvalidOffset = -1;

	// Offset is a pseudonym of RVA (relative virtual address)
	using Offset = size_t;

	// Complex offset consists of {byte offset} and {index} parts (used for PCode instructions)
	class ComplexOffset
	{
		union {
			struct {
				size_t m_index : 8;
				size_t m_byteOffset : 56;
			};
			struct {
				size_t m_fullOffset : 64;
			};
		};

	public:
		ComplexOffset(size_t fullOffset = InvalidOffset);
		
		ComplexOffset(Offset byteOffset, size_t index);

		size_t getIndex() const;

		Offset getByteOffset() const;

		operator size_t() const;
	};
};