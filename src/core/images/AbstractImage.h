#pragma once
#include <list>
#include <stdexcept>

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

		uint64_t getMinOffset() const {
			return m_pointerToRawData;
		}

		uint64_t getMaxOffset() const {
			return m_pointerToRawData + m_virtualSize;
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

		const std::list<ImageSection>& getImageSections()
		{
			return m_imageSections;
		}

		uint64_t addrToRva(uint64_t addr) {
			return addr - getAddress();
		}

		// rva to file offset (ghidra makes this transform automatically)
		uint64_t toImageOffset(uint64_t rva) {
			const auto section = getSectionByRva(rva);
			if (!section)
				return rva;
			return rva - section->m_relVirtualAddress + section->m_pointerToRawData;
		}

		const ImageSection* getSectionByRva(uint64_t rva)
		{
			for (auto& section : m_imageSections)
			{
				if (rva >= section.m_relVirtualAddress && rva < section.m_relVirtualAddress + section.m_virtualSize)
				{
					return &section;
				}
			}
			return &DefaultSection;
		}
	};
};