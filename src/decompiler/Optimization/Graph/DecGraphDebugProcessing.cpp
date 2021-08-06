#include "DecGraphDebugProcessing.h"

using namespace CE::Decompiler::Optimization;

GraphDebugProcessing::GraphDebugProcessing(DecompiledCodeGraph* decGraph, bool seqLines)
	: GraphModification(decGraph), m_seqLines(seqLines) {
}

void GraphDebugProcessing::start() {
	if(m_seqLines) {
		// seq lines (after lines expanding)
		for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
			ProcessLines(decBlock->getSeqAssignmentLines());
		}
	}
	else {
		// parallel lines (before lines expanding)
		for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
			ProcessLines(decBlock->getSymbolParallelAssignmentLines());
		}
	}
}

void GraphDebugProcessing::ProcessLines(std::list<DecBlock::AssignmentLine*>& lines) {
	// calculate
	for (const auto line : lines) {
		if (line->m_lastRequiredInstruction)
			continue;
		std::list<Instruction*> instructions;
		GetInstructions(line->getNode(), instructions);
		if (!instructions.empty()) {
			Sort(instructions); // todo: remove (implement max search)
			line->m_lastRequiredInstruction = *instructions.rbegin();
		}
	}

	// sort
	lines.sort([&](DecBlock::AssignmentLine* seqLine1, DecBlock::AssignmentLine* seqLine2)
	{
		return GetOrder(seqLine1) < GetOrder(seqLine2);
	});
}

uint64_t GraphDebugProcessing::GetOrder(DecBlock::AssignmentLine* seqLine) {
	const auto lastInstr = seqLine->getLastReqInstr();
	if (!lastInstr)
		return 0xFFFFFFFFFFFFFFFF;
	return lastInstr->getOffset();
}
