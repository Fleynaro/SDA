#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	// process the graph for debugging
	class GraphDebugProcessing : public GraphModification
	{
	public:
		GraphDebugProcessing(DecompiledCodeGraph* decGraph)
			: GraphModification(decGraph)
		{}

		void start() override {
			calculateLastReqInstructions();
			sort();
		}

	private:
		void calculateLastReqInstructions() const {
			for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
				for (const auto seqLine : decBlock->getSeqAssignmentLines()) {
					std::list<Instruction*> instructions;
					GetInstructions(seqLine->getNode(), instructions);
					if (!instructions.empty()) {
						Sort(instructions); // todo: remove (implement max search)
						seqLine->m_lastRequiredInstruction = *instructions.rbegin();
					}
				}
			}
		}

		void sort() const {
			for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
				decBlock->getSeqAssignmentLines().sort([&](DecBlock::SeqAssignmentLine* seqLine1, DecBlock::SeqAssignmentLine* seqLine2)
					{
						return GetOrder(seqLine1) < GetOrder(seqLine2);
					});
			}
		}

		static uint64_t GetOrder(DecBlock::SeqAssignmentLine* seqLine) {
			const auto lastInstr = seqLine->m_lastRequiredInstruction;
			if (!lastInstr)
				return 0xFFFFFFFFFFFFFFFF;
			return lastInstr->getOffset();
		}
	};
};