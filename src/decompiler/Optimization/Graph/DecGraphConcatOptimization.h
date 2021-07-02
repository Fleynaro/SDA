#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	class GraphConcatOptimization : public GraphModification
	{
	public:
		GraphConcatOptimization(DecompiledCodeGraph* decGraph);

		void start() override;
	private:
		void findConcat(INode* node);
	};
};