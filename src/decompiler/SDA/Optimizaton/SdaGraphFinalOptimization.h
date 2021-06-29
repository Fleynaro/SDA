#pragma once
#include "SdaGraphMemoryOptimization.h"
#include "SdaGraphUselessLineOptimization.h"

namespace CE::Decompiler::Optimization
{
	static void MakeFinalGraphOptimization(SdaCodeGraph* sdaCodeGraph) {
		Optimization::SdaGraphMemoryOptimization memoryOptimization(sdaCodeGraph);
		memoryOptimization.start();

		Optimization::SdaGraphUselessLineOptimization uselessLineOptimization(sdaCodeGraph);
		uselessLineOptimization.start();
	}
};