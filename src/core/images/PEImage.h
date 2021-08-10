#pragma once
#include "AbstractImage.h"
#include <Windows.h>

namespace CE
{
	class PEImage : public AbstractImage
	{
		IMAGE_DOS_HEADER m_imgDosHeader;
		IMAGE_NT_HEADERS m_imgNtHeaders;
	public:
		PEImage(IReader* reader);

		~PEImage();

		void analyze();

		int getOffsetOfEntryPoint() override;

		std::uintptr_t getAddress() override;

	private:
		void parse();

		void loadSections();
	};
};