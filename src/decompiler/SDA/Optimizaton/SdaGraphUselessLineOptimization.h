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
		SdaGraphUselessLineOptimization(SdaCodeGraph* sdaCodeGraph);

		void start() override;

	private:
		//set of the symbols that are used appearing in various places
		std::set<CE::Symbol::ISymbol*> m_usedSdaSymbols;
		std::set<CE::Symbol::ISymbol*> m_prevUsedSdaSymbols;
		bool m_isFirstPass = true;
		DecBlock::SeqAssignmentLine* m_curSeqLine = nullptr;

		void defineUsedSdaSymbols(INode* node, DecBlock::SeqAssignmentLine* curSeqLine);

		// funcVar1 = func1() where "funcVar1" is unused anywhere
		bool isSeqLineUseless(DecBlock::SeqAssignmentLine* seqLine);

		// funcVar1 = func1() or memVar1 = *(float*)&globalVar1
		bool isSeqLineSuit(DecBlock::SeqAssignmentLine* seqLine, SdaSymbolLeaf*& sdaDstSymbolLeaf);
	};
};