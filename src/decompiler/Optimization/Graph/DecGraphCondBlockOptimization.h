#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	//Transforming sequance of simple blocks to single complex block with complex condition(&&, ||) for jump
	class GraphCondBlockOptimization : public GraphModification
	{
	public:
		GraphCondBlockOptimization(DecompiledCodeGraph* decGraph);

		void start() override;
	private:
		void doBlockJoining();

		void optimizeConditionDecBlock(DecBlock* block);

		DecBlock* joinCondition(DecBlock* block);
	};
};