#pragma once
#include "../SdaGraphModification.h"

namespace CE::Decompiler::Optimization
{
	using namespace ExprTree;

	/*
		localVar2 = param1
		localVar3 = localVar2

		if "localVar3" is unused anywhere then "localVar2" is also useless
	*/

	class SdaGraphUselessLineOptimization : public SdaGraphModification
	{
	public:
		SdaGraphUselessLineOptimization(SdaCodeGraph* sdaCodeGraph)
			: SdaGraphModification(sdaCodeGraph)
		{}

		void start() override {
			//gather all USED symbols through multiple iterations
			while (m_isFirstPass || m_usedSdaSymbols.size() != m_prevUsedSdaSymbols.size()) {
				if (!m_isFirstPass) {
					// swaping
					m_prevUsedSdaSymbols = m_usedSdaSymbols;
					m_usedSdaSymbols.clear();
				}
				passAllTopNodes([&](DecBlock::BlockTopNode* topNode) {
					auto curSeqLine = dynamic_cast<DecBlock::SeqAssignmentLine*>(topNode);
					defineUsedSdaSymbols(topNode->getNode(), curSeqLine);
					});
				m_isFirstPass = false;
			}

			//try deleting all lines that contains unused symbol as a destination (localVar1 = 10, but "localVar1" is unused)
			passAllTopNodes([&](DecBlock::BlockTopNode* topNode) {
				if (auto seqLine = dynamic_cast<DecBlock::SeqAssignmentLine*>(topNode)) {
					if (isSeqLineUseless(seqLine))
						delete seqLine;
				}
				});
		}

	private:
		//set of the symbols that are used appearing in various places
		std::set<CE::Symbol::ISymbol*> m_usedSdaSymbols;
		std::set<CE::Symbol::ISymbol*> m_prevUsedSdaSymbols;
		bool m_isFirstPass = true;
		DecBlock::SeqAssignmentLine* m_curSeqLine = nullptr;

		void defineUsedSdaSymbols(INode* node, DecBlock::SeqAssignmentLine* curSeqLine) {
			node->iterateChildNodes([&](INode* childNode) {
				defineUsedSdaSymbols(childNode, curSeqLine);
				});

			//we need sda symbol leafs only
			auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(node);
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
		bool isSeqLineUseless(DecBlock::SeqAssignmentLine* seqLine) {
			SdaSymbolLeaf* sdaDstSymbolLeaf;
			if (isSeqLineSuit(seqLine, sdaDstSymbolLeaf)) {
				if (m_usedSdaSymbols.find(sdaDstSymbolLeaf->getSdaSymbol()) == m_usedSdaSymbols.end()) { //unused?
					return true;
				}
			}
			return false;
		}

		// funcVar1 = func1() or memVar1 = *(float*)&globalVar1
		bool isSeqLineSuit(DecBlock::SeqAssignmentLine* seqLine, SdaSymbolLeaf*& sdaDstSymbolLeaf) {
			if (auto sdaGenericNode = dynamic_cast<SdaGenericNode*>(seqLine->getNode())) {
				if (auto assignmentNode = dynamic_cast<AssignmentNode*>(sdaGenericNode->getNode())) {
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
	};
};