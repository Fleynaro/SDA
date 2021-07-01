#pragma once
#include "AbstractSymbol.h"
#include <decompiler/DecStorage.h>

namespace CE::Symbol
{
	class IMemorySymbol : virtual public ISymbol
	{
	public:
		virtual Decompiler::Storage getStorage() = 0;
	};
};