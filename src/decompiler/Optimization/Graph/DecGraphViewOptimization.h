#pragma once
#include "DecGraphModification.h"
#include "../ExprOptimization.h"

namespace CE::Decompiler::Optimization
{
	//optimizing the graph to be more understanding by human intelligence
	class GraphViewOptimization : public GraphModification
	{
	public:
		GraphViewOptimization(DecompiledCodeGraph* decGraph);

		void start() override;
	private:
		void processBlock(DecBlock* block);

		//gather localVars that store nodes according to the filter
		void gatherLocalVarsDependedOnItselfFromBlock(DecBlock* block, std::map<HS::Value, Symbol::LocalVariable*>& nodeHashTolocalVar);
		
		void replaceConfusedNodesWithGatheredLocalVars(INode* node, std::map<HS::Value, Symbol::LocalVariable*>& nodeHashTolocalVar);

		//select simple nodes like symbols, numbers, ...
		bool filter(INode* node);
	};
};