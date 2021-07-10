#pragma once
#include "AbstractImage.h"
#include <Windows.h>

namespace CE
{
	class PEImage : public AbstractImage
	{
		int8_t* m_data;
		int m_size;
		PIMAGE_DOS_HEADER m_pImgDosHeader;
		PIMAGE_NT_HEADERS m_pImgNtHeaders;
		PIMAGE_SECTION_HEADER m_pImgSecHeader;
	public:
		PEImage(int8_t* data, int size);

		int8_t* getData() override;

		int getSize() override;

		int getOffsetOfEntryPoint() override;

		std::uintptr_t getAddress() override;

	private:
		void parse();

		void loadSections();
	};
};