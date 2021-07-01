#pragma once
#include "IImage.h"
#include <Windows.h>

namespace CE
{
	class PEImage : public IImage
	{
		int8_t* m_data;
		int m_size;
		PIMAGE_NT_HEADERS m_pImgNtHeaders;
		PIMAGE_SECTION_HEADER m_pImgSecHeader;
	public:
		PEImage(int8_t* data, int size)
			: m_data(data), m_size(size)
		{
			parse();
		}

		int8_t* getData() override;

		int getSize() override;

		int getOffsetOfEntryPoint() override;

		SegmentType defineSegment(int offset) override;

		// rva to file offset (ghidra makes this transform automatically)
		int toImageOffset(int offset) override;

		// virtual address to file offset
		int addrToImageOffset(uint64_t addr) override;

		std::uintptr_t getAddress() override;

	private:
		PIMAGE_SECTION_HEADER defineSection(DWORD rva);

		void parse();
	};
};