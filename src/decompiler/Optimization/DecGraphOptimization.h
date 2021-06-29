#pragma once
#include "Graph/DecGraphCondBlockOptimization.h"
#include "Graph/DecGraphExprOptimization.h"
#include "Graph/DecGraphLinesExpanding.h"
#include "Graph/DecGraphUselessLineOptimization.h"
#include "Graph/DecGraphLastLineAndConditionOrderFixing.h"
#include "Graph/DecGraphViewOptimization.h"
#include "Graph/DecGraphParAssignmentCreator.h"

namespace CE::Decompiler::Optimization
{
	// make full optimization of decompiled graph
	static void ProcessDecompiledGraph(DecompiledCodeGraph* decGraph, PrimaryDecompiler* decompiler)
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
};