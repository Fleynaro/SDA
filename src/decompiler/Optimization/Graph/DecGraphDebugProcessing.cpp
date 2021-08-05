#include "DecGraphDebugProcessing.h"

using namespace CE::Decompiler::Optimization;

GraphDebugProcessing::GraphDebugProcessing(DecompiledCodeGraph* decGraph):
	GraphModification(decGraph) {
}

void GraphDebugProcessing::start() {
	calculateLastReqInstructions();
	sortSeqLines();
}

void GraphDebugProcessing::calculateLastReqInstructions() const {
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

void GraphDebugProcessing::sortSeqLines() const {
	for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
		decBlock->getSeqAssignmentLines().sort(
			[&](DecBlock::SeqAssignmentLine* seqLine1, DecBlock::SeqAssignmentLine* seqLine2)
			{
				return GetOrder(seqLine1) < GetOrder(seqLine2);
			});
	}
}

uint64_t GraphDebugProcessing::GetOrder(DecBlock::SeqAssignmentLine* seqLine) {
	const auto lastInstr = seqLine->getLastReqInstr();
	if (!lastInstr)
		return 0xFFFFFFFFFFFFFFFF;
	return lastInstr->getOffset();
}
