#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	// process the graph for debugging
	class GraphDebugProcessing : public GraphModification
	{
		bool m_seqLines;
	public:
		GraphDebugProcessing(DecompiledCodeGraph* decGraph, bool seqLines);

		void start() override;

	private:
		static void ProcessLines(std::list<DecBlock::AssignmentLine*>& lines);

		static uint64_t GetOrder(DecBlock::AssignmentLine* seqLine);
	};
};