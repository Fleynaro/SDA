#include "DecGraphUselessLineOptimization.h"

CE::Decompiler::Optimization::GraphUselessLineDeleting::GraphUselessLineDeleting(DecompiledCodeGraph* decGraph)
	: GraphModification(decGraph)
{}

void CE::Decompiler::Optimization::GraphUselessLineDeleting::start() {
	for (auto decBlock : m_decGraph->getDecompiledBlocks()) {
		processBlock(decBlock);
	}
}

void CE::Decompiler::Optimization::GraphUselessLineDeleting::processBlock(DecBlock* block) {
	for (auto seqLine : block->getSeqAssignmentLines()) {
		if (const auto symbolLeaf = dynamic_cast<SymbolLeaf*>(seqLine->getDstNode()))
			if (dynamic_cast<Symbol::AbstractVariable*>(symbolLeaf->m_symbol))
				continue;

		if (hasUnknownRegister(seqLine->getAssignmentNode())) {
			delete seqLine;
		}
	}
}

bool CE::Decompiler::Optimization::GraphUselessLineDeleting::hasUnknownRegister(INode* node) {
	if (dynamic_cast<FunctionCall*>(node))
		return false;

	bool result = false;
	node->iterateChildNodes([&](INode* childNode) {
		if (!result)
			result = hasUnknownRegister(childNode);
		});
	if (result)
		return true;

	if (const auto symbolLeaf = dynamic_cast<SymbolLeaf*>(node)) {
		if (auto regVar = dynamic_cast<Symbol::RegisterVariable*>(symbolLeaf->m_symbol)) {
			if (regVar->m_register.isPointer())
				return false;
			return isUnknownRegister(regVar->m_register);
		}
	}
	return false;
}

bool CE::Decompiler::Optimization::GraphUselessLineDeleting::isUnknownRegister(Register& reg) {
	const bool isFound = false;
	/*for (auto paramInfo : m_decGraph->getFunctionCallInfo().getParamInfos()) {
	if (reg.getId() == paramInfo.m_storage.getRegisterId()) {
	isFound = true;
	break;
	}
	}*/
	return !isFound;
}
