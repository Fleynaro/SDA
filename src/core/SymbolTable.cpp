#include "SymbolTable.h"

using namespace CE;
using namespace CE::Symbol;

CE::Symbol::SymbolTable::SymbolTable(SymbolTableManager* manager, SymbolTableType type)
	: m_manager(manager), m_type(type)
{}

SymbolTableManager* SymbolTable::getManager() const
{
	return m_manager;
}

SymbolTable::SymbolTableType SymbolTable::getType() const
{
	return m_type;
}

void SymbolTable::addSymbol(AbstractSymbol* symbol, int64_t offset) {
	m_symbols.insert(std::make_pair(offset, symbol));
}

std::pair<int64_t, AbstractSymbol*> SymbolTable::getSymbolAt(int64_t offset) {
	auto it = getSymbolIterator(offset);
	if (it != m_symbols.end())
		return std::make_pair(it->first, it->second);
	return std::make_pair(0, nullptr);
}

std::map<int64_t, AbstractSymbol*>::iterator SymbolTable::getSymbolIterator(int64_t offset) {
	if (!m_symbols.empty()) {
		auto it = std::prev(m_symbols.upper_bound(offset));
		if (it != m_symbols.end()) {
			const auto symbolOffset = it->first;
			auto symbol = it->second;
			if (offset < symbolOffset + symbol->getSize()) {
				return it;
			}
		}
	}
	return m_symbols.end();
}

std::map<int64_t, AbstractSymbol*>& SymbolTable::getSymbols() {
	return m_symbols;
}
