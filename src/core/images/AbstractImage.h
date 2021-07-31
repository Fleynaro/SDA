#pragma once
#include <list>
#include <stdexcept>
#include "Offset.h"

namespace CE
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
		uint64_t m_relVirtualAddress = 0;
		uint64_t m_virtualSize = 0;
		uint64_t m_pointerToRawData = m_virtualSize;

		Offset getMinOffset() const {
			return m_relVirtualAddress;
		}

		Offset getMaxOffset() const {
			return m_relVirtualAddress + m_virtualSize;
		}

		bool contains(Offset offset) const {
			return offset >= getMinOffset() && offset < getMaxOffset();
		}

		// image file offset to rva(=offset) (ghidra makes this transform automatically)
		Offset toOffset(uint64_t offset) const {
			return offset - m_pointerToRawData + m_relVirtualAddress;
		}

		// rva(=offset) to image file offset (ghidra makes this transform automatically)
		uint64_t toImageOffset(Offset offset) const {
			return offset - m_relVirtualAddress + m_pointerToRawData;
		}
	};
	
	// raw image that can manipualtes with bytes, no symbols or other high-level things
	class AbstractImage
	{
		inline const static ImageSection DefaultSection = ImageSection();
	protected:
		std::list<ImageSection> m_imageSections;
	public:
		virtual int8_t* getData() = 0;

		virtual int getSize() = 0;

		virtual int getOffsetOfEntryPoint() = 0;

		virtual std::uintptr_t getAddress() {
			return 0x0;
		}

		const std::list<ImageSection>& getImageSections() const
		{
			return m_imageSections;
		}

		uint64_t addrToRva(uint64_t addr) {
			return addr - getAddress();
		}

		// rva(=offset) to image file offset (ghidra makes this transform automatically)
		uint64_t toImageOffset(Offset offset) const {
			const auto section = getSectionByOffset(offset);
			if (!section)
				return offset;
			return section->toImageOffset(offset);
		}

		const ImageSection* getSectionByOffset(Offset offset) const {
			for (auto& section : m_imageSections) {
				if (section.contains(offset)) {
					return &section;
				}
			}
			return &DefaultSection;
		}
	};
};