#pragma once
#include "AbstractImage.h"
#include <vector>

namespace CE
{
	class VectorBufferImage : public AbstractImage
	{
		std::vector<int8_t> m_content;
	public:
		VectorBufferImage(std::vector<int8_t> content);

		int8_t* getData() override;

		int getSize() override;

		int getOffsetOfEntryPoint() override;
	};
};