#include "DecGraphOptimization.h"

// make full optimization of decompiled graph
void CE::Decompiler::Optimization::ProcessDecompiledGraph(DecompiledCodeGraph* decGraph, PrimaryDecompiler* decompiler)
{
	decGraph->cloneAllExpr();

	GraphCondBlockOptimization graphCondBlockOptimization(decGraph);
	graphCondBlockOptimization.start();

	GraphExprOptimization graphExprOptimization(decGraph);
	graphExprOptimization.start();

	GraphParAssignmentCreator graphParAssignmentCreator(decGraph, decompiler);
	graphParAssignmentCreator.start();

	GraphLastLineAndConditionOrderFixing graphLastLineAndConditionOrderFixing(decGraph);
	graphLastLineAndConditionOrderFixing.start();

	GraphViewOptimization graphViewOptimization(decGraph);
	graphViewOptimization.start();

	GraphLinesExpanding graphLinesExpanding(decGraph);
	graphLinesExpanding.start();

	GraphUselessLineDeleting GraphUselessLineDeleting(decGraph);
	//GraphUselessLineDeleting.start();

	DecompiledCodeGraph::CalculateHeightForDecBlocks(decGraph->getStartBlock());
}
