#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	// process the graph for debugging
	class GraphDebugProcessing : public GraphModification
	{
	public:
		GraphDebugProcessing(DecompiledCodeGraph* decGraph);

		void start() override;

	private:
		void calculateLastReqInstructions() const;

		void sortSeqLines() const;

		static uint64_t GetOrder(DecBlock::SeqAssignmentLine* seqLine);
	};
};