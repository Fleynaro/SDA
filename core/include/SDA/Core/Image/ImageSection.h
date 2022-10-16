#pragma once
#include <string>
#include "SDA/Core/Offset.h"

namespace sda
{
    struct ImageSection
	{
		enum SegmentType {
			NONE_SEGMENT,
			CODE_SEGMENT,
			DATA_SEGMENT
		};

		std::string m_name = "None";
		SegmentType m_type = NONE_SEGMENT;
		size_t m_relVirtualAddress = 0;
		size_t m_virtualSize = 0;
		size_t m_pointerToRawData = 0;

		Offset getMinOffset() const;

		Offset getMaxOffset() const;

		bool contains(Offset offset) const;

		// Image file offset to rva(=offset) (ghidra makes this transform automatically)
		Offset toOffset(size_t offset) const;

		// Rva(=offset) to image file offset (ghidra makes this transform automatically)
		size_t toImageFileOffset(Offset offset) const;
	};
};