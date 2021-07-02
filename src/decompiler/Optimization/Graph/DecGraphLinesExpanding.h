#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	/*
		Before (parallel assignments):
			localVar1 = 1
			localVar2 = localVar1 + 1
		After (sequance assignments):
			tempVar1 = localVar1
			localVar1 = 1
			localVar2 = tempVar1 + 1
	*/

	// transforming parallel assignments to sequance assignments
	class GraphLinesExpanding : public GraphModification
	{
	public:
		GraphLinesExpanding(DecompiledCodeGraph* decGraph);

		void start() override;
	private:
		void processBlock(DecBlock* block);
	};
};