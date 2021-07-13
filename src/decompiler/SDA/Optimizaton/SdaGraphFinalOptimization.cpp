#include "SdaGraphFinalOptimization.h"
#include "SdaGraphMemoryOptimization.h"
#include "SdaGraphUselessLineOptimization.h"

void CE::Decompiler::Optimization::MakeFinalGraphOptimization(SdaCodeGraph* sdaCodeGraph) {
	SdaGraphMemoryOptimization memoryOptimization(sdaCodeGraph);
	memoryOptimization.start();

	SdaGraphUselessLineOptimization uselessLineOptimization(sdaCodeGraph);
	uselessLineOptimization.start();
}
