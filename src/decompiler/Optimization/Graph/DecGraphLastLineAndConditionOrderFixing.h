#pragma once
#include "DecGraphModification.h"
#include "../ExprOptimization.h"

namespace CE::Decompiler::Optimization
{
	// Replace in conditions: {localVar + 1} = > {localVar} (localVar used in last assignments as increment: localVar = {localVar + 1})
	class GraphLastLineAndConditionOrderFixing : public GraphModification
	{
	public:
		GraphLastLineAndConditionOrderFixing(DecompiledCodeGraph* decGraph);

		void start() override;
	private:
		void processBlock(DecBlock* block);

		//gather localVars located in something like this: localVar = localVar + 1
		void gatherLocalVarsDependedOnItselfFromBlock(DecBlock* block, std::map<HS::Value, Symbol::LocalVariable*>& localVars);

		//replace in condition: {localVar + 1}	=>	 {localVar}
		bool doSingleFix(INode* node, std::map<HS::Value, Symbol::LocalVariable*>& localVars);
	};
};