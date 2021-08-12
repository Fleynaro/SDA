#pragma once
#include "AbstractImage.h"

namespace CE
{
	class SimpleImage : public AbstractImage
	{
		std::uintptr_t m_address;
	public:
		SimpleImage(IReader* reader);

		int getOffsetOfEntryPoint() override;

		std::uintptr_t getAddress() override;
	};
};