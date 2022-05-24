#pragma once

namespace sda
{
	// Offset is a pseudonym of RVA (relative virtual address)
	using Offset = size_t;

	inline const static Offset InvalidOffset = -1;
};