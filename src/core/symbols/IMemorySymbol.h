#pragma once
#include "AbstractSymbol.h"
#include <Decompiler/DecStorage.h>

namespace CE::Symbol
{
	class IMemorySymbol : virtual public ISymbol
	{
	public:
		virtual Decompiler::Storage getStorage() = 0;
	};
};