#include "SdaGraphFinalOptimization.h"
#include "SdaGraphMemoryOptimization.h"
#include "SdaGraphUselessLineOptimization.h"

void CE::Decompiler::Optimization::MakeFinalGraphOptimization(SdaCodeGraph* sdaCodeGraph) {
	Optimization::SdaGraphMemoryOptimization memoryOptimization(sdaCodeGraph);
	memoryOptimization.start();

	Optimization::SdaGraphUselessLineOptimization uselessLineOptimization(sdaCodeGraph);
	uselessLineOptimization.start();
}
