#include "DecGraphLastLineAndConditionOrderFixing.h"

using namespace CE::Decompiler;

//replace in condition: {localVar + 1}	=>	 {localVar}

Optimization::GraphLastLineAndConditionOrderFixing::GraphLastLineAndConditionOrderFixing(DecompiledCodeGraph* decGraph)
	: GraphModification(decGraph)
{}

void Optimization::GraphLastLineAndConditionOrderFixing::start() {
	for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
		if (decBlock->getNoJumpCondition()) {
			processBlock(decBlock);
		}
	}
}

void Optimization::GraphLastLineAndConditionOrderFixing::processBlock(DecBlock* block) {
	std::map<HS::Value, Symbol::LocalVariable*> localVars;
	gatherLocalVarsDependedOnItselfFromBlock(block, localVars);
	doSingleFix(block->getNoJumpCondition(), localVars);
}

//gather localVars located in something like this: localVar = localVar + 1

void Optimization::GraphLastLineAndConditionOrderFixing::gatherLocalVarsDependedOnItselfFromBlock(DecBlock* block, std::map<HS::Value, Symbol::LocalVariable*>& localVars) {
	for (auto symbolAssignmentLine : block->getSymbolParallelAssignmentLines()) {
		if (auto localVar = dynamic_cast<Symbol::LocalVariable*>(symbolAssignmentLine->getDstSymbolLeaf()->m_symbol)) {
			//if localVar expressed through itself (e.g. localVar = {localVar + 1})
			if (DoesNodeHaveSymbol(symbolAssignmentLine->getSrcNode(), localVar)) {
				// grab right node (e.g. {localVar + 1})
				localVars.insert(std::make_pair(symbolAssignmentLine->getSrcNode()->getHash().getHashValue(), localVar));
			}
		}
	}
}

bool Optimization::GraphLastLineAndConditionOrderFixing::doSingleFix(INode* node, std::map<HS::Value, Symbol::LocalVariable*>& localVars) {
	const auto it = localVars.find(node->getHash().getHashValue());
	if (it != localVars.end()) {
		node->replaceWith(new SymbolLeaf(it->second));
		delete node;
		return true;
	}

	bool result = false;
	node->iterateChildNodes([&](INode* childNode) {
		if (!result)
			result = doSingleFix(childNode, localVars);
		});
	return result;
}
