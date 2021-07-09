#include "DecCodeGraph.h"

using namespace CE::Decompiler;

// pass decompiled graph and calculate max distance from the root to each node (dec block). Similarly to asm graph!

CE::Decompiler::DecompiledCodeGraph::DecompiledCodeGraph(FunctionPCodeGraph* funcGraph)
	: m_funcGraph(funcGraph)
{}

CE::Decompiler::DecompiledCodeGraph::~DecompiledCodeGraph() {
	for (auto block : m_decompiledBlocks) {
		delete block;
	}

	for (auto block : m_removedDecompiledBlocks) {
		delete block;
	}

	for (auto symbol : getSymbols()) {
		delete symbol;
	}
}

FunctionPCodeGraph* CE::Decompiler::DecompiledCodeGraph::getFuncGraph() const
{
	return m_funcGraph;
}

DecBlock* CE::Decompiler::DecompiledCodeGraph::getStartBlock() {
	return *getDecompiledBlocks().begin();
}

std::list<DecBlock*>& CE::Decompiler::DecompiledCodeGraph::getDecompiledBlocks() {
	return m_decompiledBlocks;
}

std::list<Symbol::Symbol*>& CE::Decompiler::DecompiledCodeGraph::getSymbols() {
	return m_symbols;
}

void CE::Decompiler::DecompiledCodeGraph::removeDecompiledBlock(DecBlock* decBlock) {
	m_decompiledBlocks.remove(decBlock);
	m_removedDecompiledBlocks.push_back(decBlock);
	decBlock->disconnect();
}

void CE::Decompiler::DecompiledCodeGraph::addSymbol(Symbol::Symbol* symbol) {
	m_symbols.push_back(symbol);
}

void CE::Decompiler::DecompiledCodeGraph::removeSymbol(Symbol::Symbol* symbol) {
	m_symbols.remove(symbol);
}

void CE::Decompiler::DecompiledCodeGraph::cloneAllExpr() {
	for (auto block : m_decompiledBlocks) {
		block->cloneAllExpr();
	}
}

void CE::Decompiler::DecompiledCodeGraph::sortBlocksByLevel() {
	m_decompiledBlocks.sort([](DecBlock* a, DecBlock* b) {
		return a->m_level < b->m_level;
		});
}

void CE::Decompiler::DecompiledCodeGraph::checkOnSingleParents() {
	for (const auto decBlock : getDecompiledBlocks()) {
		for (auto topNode : decBlock->getAllTopNodes()) {
			ExprTree::INode::UpdateDebugInfo(topNode->getNode());
			topNode->getNode()->checkOnSingleParents();
		}
	}
}

HS CE::Decompiler::DecompiledCodeGraph::getHash() {
	HS hs;
	for (const auto decBlock : getDecompiledBlocks()) {
		for (auto topNode : decBlock->getAllTopNodes()) {
			hs = hs << topNode->getNode()->getHash();
		}
	}
	return hs;
}

// recalculate levels because some blocks can be removed (while parsing AND/OR block constructions)

void CE::Decompiler::DecompiledCodeGraph::recalculateLevelsForBlocks() {
	for (const auto decBlock : getDecompiledBlocks()) {
		decBlock->m_level = 0;
	}
	std::list<DecBlock*> path;
	DecompiledCodeGraph::CalculateLevelsForDecBlocks(getStartBlock(), path);
}

// calculate count of lines(height) for each block beginining from lower blocks (need as some score for linearization)

int CE::Decompiler::DecompiledCodeGraph::CalculateHeightForDecBlocks(DecBlock* block) {
	int height = 0;
	for (auto nextBlock : block->getNextBlocks()) {
		if (nextBlock->m_level > block->m_level) { // to avoid loops
			auto h = CalculateHeightForDecBlocks(nextBlock);
			height = std::max(height, h);
		}
	}
	block->m_maxHeight = height + static_cast<int>(block->getSeqAssignmentLines().size());
	return block->m_maxHeight;
}

void CE::Decompiler::DecompiledCodeGraph::CalculateLevelsForDecBlocks(DecBlock* block, std::list<DecBlock*>& path) {
	if (block == nullptr)
		return;

	//check if there's a loop
	for (auto it = path.rbegin(); it != path.rend(); it++) {
		if (block == *it) {
			return;
		}
	}

	path.push_back(block);
	block->m_level = std::max(block->m_level, static_cast<int>(path.size()));
	CalculateLevelsForDecBlocks(block->getNextNearBlock(), path);
	CalculateLevelsForDecBlocks(block->getNextFarBlock(), path);
	path.pop_back();
}
