#pragma once
#include "AbstractImage.h"

namespace CE
{
	class SimpleImage : public AbstractImage
	{
		int m_size;
	public:
		SimpleImage(IReader* reader, int size);

		int getOffsetOfEntryPoint() override;
	};
};