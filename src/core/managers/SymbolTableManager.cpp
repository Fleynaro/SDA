#include "SymbolTableManager.h"
#include <database/Mappers/SymbolTableMapper.h>

using namespace CE;
using namespace Symbol;

SymbolTableManager::SymbolTableManager(Project* module)
	: AbstractItemManager(module)
{
	m_symbolTableMapper = new DB::SymbolTableMapper(this);
}

void SymbolTableManager::loadSymTables() {
	m_symbolTableMapper->loadAll();
}

SymbolTableManager::Factory CE::SymbolTableManager::getFactory(bool markAsNew) {
	return Factory(this, m_symbolTableMapper, markAsNew);
}

SymbolTable* SymbolTableManager::findSymbolTableById(DB::Id id) {
	return dynamic_cast<SymbolTable*>(find(id));
}

Symbol::SymbolTable* CE::SymbolTableManager::Factory::createSymbolTable(Symbol::SymbolTable::SymbolTableType type) {
	auto symbol = new SymbolTable(m_symbolTableManager, type);
	symbol->setMapper(m_symbolTableMapper);
	if (m_markAsNew)
		m_symbolTableManager->getProject()->getTransaction()->markAsNew(symbol);
	return symbol;
}
