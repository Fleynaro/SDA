#include "SdaGraphUselessLineOptimization.h"

// funcVar1 = func1() or memVar1 = *(float*)&globalVar1

CE::Decompiler::Optimization::SdaGraphUselessLineOptimization::SdaGraphUselessLineOptimization(SdaCodeGraph* sdaCodeGraph)
	: SdaGraphModification(sdaCodeGraph)
{}

void CE::Decompiler::Optimization::SdaGraphUselessLineOptimization::start() {
	//gather all USED symbols through multiple iterations
	while (m_isFirstPass || m_usedSdaSymbols.size() != m_prevUsedSdaSymbols.size()) {
		if (!m_isFirstPass) {
			// swaping
			m_prevUsedSdaSymbols = m_usedSdaSymbols;
			m_usedSdaSymbols.clear();
		}
		passAllTopNodes([&](DecBlock::BlockTopNode* topNode) {
			const auto curSeqLine = dynamic_cast<DecBlock::AssignmentLine*>(topNode);
			defineUsedSdaSymbols(topNode->getNode(), curSeqLine);
			});
		m_isFirstPass = false;
	}

	//try deleting all lines that contains unused symbol as a destination (localVar1 = 10, but "localVar1" is unused)
	passAllTopNodes([&](DecBlock::BlockTopNode* topNode) {
		if (const auto seqLine = dynamic_cast<DecBlock::AssignmentLine*>(topNode)) {
			if (isSeqLineUseless(seqLine))
				delete seqLine;
		}
		});
}

void CE::Decompiler::Optimization::SdaGraphUselessLineOptimization::defineUsedSdaSymbols(INode* node, DecBlock::AssignmentLine* curSeqLine) {
	node->iterateChildNodes([&](INode* childNode) {
		defineUsedSdaSymbols(childNode, curSeqLine);
		});

	//we need sda symbol leafs only
	const auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(node);
	if (!sdaSymbolLeaf)
		return;
	//memVar, funcVar, localVar
	if (sdaSymbolLeaf->getSdaSymbol()->getType() != CE::Symbol::LOCAL_INSTR_VAR)
		return;

	if (curSeqLine) {
		SdaSymbolLeaf* sdaDstSymbolLeaf;
		if (isSeqLineSuit(curSeqLine, sdaDstSymbolLeaf)) {
			if (sdaDstSymbolLeaf->getSdaSymbol() == sdaSymbolLeaf->getSdaSymbol())
				return;
			if (!m_isFirstPass)
				if (m_prevUsedSdaSymbols.find(sdaDstSymbolLeaf->getSdaSymbol()) == m_prevUsedSdaSymbols.end())
					return;
		}
	}
	m_usedSdaSymbols.insert(sdaSymbolLeaf->getSdaSymbol());
}

// funcVar1 = func1() where "funcVar1" is unused anywhere

bool CE::Decompiler::Optimization::SdaGraphUselessLineOptimization::isSeqLineUseless(DecBlock::AssignmentLine* seqLine) {
	SdaSymbolLeaf* sdaDstSymbolLeaf;
	if (isSeqLineSuit(seqLine, sdaDstSymbolLeaf)) {
		if (m_usedSdaSymbols.find(sdaDstSymbolLeaf->getSdaSymbol()) == m_usedSdaSymbols.end()) { //unused?
			return true;
		}
	}
	return false;
}

bool CE::Decompiler::Optimization::SdaGraphUselessLineOptimization::isSeqLineSuit(DecBlock::AssignmentLine* seqLine, SdaSymbolLeaf*& sdaDstSymbolLeaf) {
	if (const auto sdaGenericNode = dynamic_cast<SdaGenericNode*>(seqLine->getNode())) {
		if (const auto assignmentNode = dynamic_cast<AssignmentNode*>(sdaGenericNode->getNode())) {
			if (sdaDstSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(assignmentNode->getDstNode())) {
				if (!dynamic_cast<SdaFunctionNode*>(assignmentNode->getSrcNode()) && //we dont touch a function call
					sdaDstSymbolLeaf->getSdaSymbol()->getType() == CE::Symbol::LOCAL_INSTR_VAR) { //memVar, funcVar, localVar
					return true;
				}
			}
		}
	}
	return false;
}
