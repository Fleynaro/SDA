#include "DecGraphViewOptimization.h"

//select simple nodes like symbols, numbers, ...

CE::Decompiler::Optimization::GraphViewOptimization::GraphViewOptimization(DecompiledCodeGraph* decGraph)
	: GraphModification(decGraph)
{}

void CE::Decompiler::Optimization::GraphViewOptimization::start() {
	for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
		processBlock(decBlock);
	}
}

void CE::Decompiler::Optimization::GraphViewOptimization::processBlock(DecBlock* block) {
	//replacing confused nodes in conditions with more short ones (e.g. if {localVar1 = localVar2 + 1} then for node {localVar2 + 1} need to be replaced with {localVar1})
	if (block->getNoJumpCondition()) {
		std::map<HS::Value, Symbol::LocalVariable*> nodeHashTolocalVar;
		gatherLocalVarsDependedOnItselfFromBlock(block, nodeHashTolocalVar);
		replaceConfusedNodesWithGatheredLocalVars(block->getNoJumpCondition(), nodeHashTolocalVar); // replacing in conditions
	}
}

//gather localVars that store nodes according to the filter

void CE::Decompiler::Optimization::GraphViewOptimization::gatherLocalVarsDependedOnItselfFromBlock(DecBlock* block, std::map<HS::Value, Symbol::LocalVariable*>& nodeHashTolocalVar) {
	for (auto symbolAssignmentLine : block->getSymbolParallelAssignmentLines()) {
		// finding something like {localVar1 = localVar2 + 1}
		if (auto localVar = dynamic_cast<Symbol::LocalVariable*>(symbolAssignmentLine->getDstSymbolLeaf()->m_symbol)) {
			// dont select simple nodes
			if (!filter(symbolAssignmentLine->getSrcNode())) {
				// get hash of {localVar2 + 1}
				nodeHashTolocalVar.insert(std::make_pair(symbolAssignmentLine->getSrcNode()->getHash().getHashValue(), localVar));
			}
		}
	}
}

void CE::Decompiler::Optimization::GraphViewOptimization::replaceConfusedNodesWithGatheredLocalVars(INode* node, std::map<HS::Value, Symbol::LocalVariable*>& nodeHashTolocalVar) {
	if (!filter(node)) {
		auto it = nodeHashTolocalVar.find(node->getHash().getHashValue());
		if (it != nodeHashTolocalVar.end()) {
			node->replaceWith(new SymbolLeaf(it->second));
			delete node;
		}
	}

	node->iterateChildNodes([&](INode* childNode) {
		replaceConfusedNodesWithGatheredLocalVars(childNode, nodeHashTolocalVar);
		});
}

bool CE::Decompiler::Optimization::GraphViewOptimization::filter(INode* node) {
	if (dynamic_cast<ILeaf*>(node))
		return true;
	return false;
}
