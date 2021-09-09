#include "DecCodeGraph.h"

using namespace CE::Decompiler;

// pass decompiled graph and calculate max distance from the root to each node (dec block). Similarly to asm graph!

DecompiledCodeGraph::DecompiledCodeGraph(FunctionPCodeGraph* funcGraph)
	: m_funcGraph(funcGraph)
{}

DecompiledCodeGraph::~DecompiledCodeGraph() {
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

FunctionPCodeGraph* DecompiledCodeGraph::getFuncGraph() const
{
	return m_funcGraph;
}

DecBlock* DecompiledCodeGraph::getStartBlock() {
	return *getDecompiledBlocks().begin();
}

std::list<DecBlock*>& DecompiledCodeGraph::getDecompiledBlocks() {
	return m_decompiledBlocks;
}

std::list<Symbol::Symbol*>& DecompiledCodeGraph::getSymbols() {
	return m_symbols;
}

void DecompiledCodeGraph::removeDecompiledBlock(DecBlock* decBlock) {
	m_decompiledBlocks.remove(decBlock);
	m_removedDecompiledBlocks.push_back(decBlock);
	decBlock->disconnect();
}

void DecompiledCodeGraph::addSymbol(Symbol::Symbol* symbol) {
	m_symbols.push_back(symbol);
}

void DecompiledCodeGraph::cloneAllExpr() {
	for (auto block : m_decompiledBlocks) {
		block->cloneAllExpr();
	}
}

void DecompiledCodeGraph::sortBlocksByLevel() {
	m_decompiledBlocks.sort([](DecBlock* a, DecBlock* b) {
		return a->m_level < b->m_level;
		});
}

void DecompiledCodeGraph::checkOnSingleParents() {
	for (const auto decBlock : getDecompiledBlocks()) {
		for (auto topNode : decBlock->getAllTopNodes()) {
			ExprTree::UpdateDebugInfo(topNode->getNode());
			topNode->getNode()->checkOnSingleParents();
		}
	}
}

HS DecompiledCodeGraph::getHash() {
	HS hs;
	for (const auto decBlock : getDecompiledBlocks()) {
		for (auto topNode : decBlock->getAllTopNodes()) {
			hs = hs << topNode->getNode()->getHash();
		}
	}
	return hs;
}

void DecompiledCodeGraph::removeNotUsedSymbols() {
	auto it = m_symbols.begin();
	while(it != m_symbols.end()) {
		const auto symbol = *it;
		if (symbol->m_parentsCount == 0) {
			for (auto& [offset, symbolValues] : m_symbolValues) {
				symbolValues.remove_if([&](const SymbolValue& value) { return symbol == value.m_symbol; });
			}
			it = m_symbols.erase(it);
			delete symbol;
		}
		else {
			++it;
		}
	}
}

DecBlock::BlockTopNode* DecompiledCodeGraph::findBlockTopNodeAtOffset(ComplexOffset offset) {
	for (auto decBlock : m_decompiledBlocks) {
		if (decBlock->m_pcodeBlock->containsOffset(offset))
			return decBlock->findBlockTopNodeByOffset(offset);
		// for complex condition
		for (auto joinedDecBlock : decBlock->m_joinedRemovedBlocks) {
			if (joinedDecBlock->m_pcodeBlock->containsOffset(offset))
				return *decBlock->getAllTopNodes().rbegin();
		}
	}
	return nullptr;
}

void DecompiledCodeGraph::addSymbolValue(ComplexOffset offset, Symbol::Symbol* symbol, const Storage& storage, bool after) {
	m_symbolValues[offset].push_back({ symbol, storage, after });
}

std::map<CE::ComplexOffset, std::list<DecompiledCodeGraph::SymbolValue>>& DecompiledCodeGraph::getSymbolValues() {
	return m_symbolValues;
}

std::map<CE::ComplexOffset, int>& DecompiledCodeGraph::getStackPointerValues() {
	return m_stackPointerValues;
}

int DecompiledCodeGraph::getStackPointerValueAtOffset(ComplexOffset offset) {
	if (m_stackPointerValues.empty())
		return 0;
	const auto it = std::prev(m_stackPointerValues.upper_bound(offset));
	if (it == m_stackPointerValues.end())
		return 0;
	return it->second;
}

// recalculate levels because some blocks can be removed (while parsing AND/OR block constructions)
void DecompiledCodeGraph::recalculateLevelsForBlocks() {
	for (const auto decBlock : getDecompiledBlocks()) {
		decBlock->m_level = 0;
	}
	std::list<DecBlock*> path;
	CalculateLevelsForDecBlocks(getStartBlock(), path);
}

// calculate count of lines(height) for each block beginining from lower blocks (need as some score for linearization)
int DecompiledCodeGraph::CalculateHeightForDecBlocks(DecBlock* block) {
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

void DecompiledCodeGraph::CalculateLevelsForDecBlocks(DecBlock* block, std::list<DecBlock*>& path) {
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
