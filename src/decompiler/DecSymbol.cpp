#include "DecSymbol.h"
#include "Graph/DecCodeGraph.h"

using namespace CE::Decompiler;
using namespace Symbol;

CE::Decompiler::Symbol::Symbol* CE::Decompiler::Symbol::Symbol::clone(ExprTree::NodeCloneContext* ctx) {
	if (!ctx->m_cloneSymbols)
		return this;
	const auto it = ctx->m_clonedSymbols.find(this);
	if (it != ctx->m_clonedSymbols.end())
		return it->second;
	auto newSymbol = cloneSymbol();
	ctx->m_clonedSymbols.insert(std::make_pair(this, newSymbol));
	return newSymbol;
}