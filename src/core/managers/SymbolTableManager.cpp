#include "SymbolTableManager.h"
#include <database/Mappers/SymbolTableMapper.h>
// todo: change include above

using namespace CE;
using namespace Symbol;

GlobalSymbolTable* SymbolTableManager::Factory::createGlobalSymbolTable() const {
	const auto symbolTable = new GlobalSymbolTable(m_symbolTableManager);
	initSymbolTable(symbolTable);
	return symbolTable;
}

StackSymbolTable* SymbolTableManager::Factory::createStackSymbolTable() const {
	const auto symbolTable = new StackSymbolTable(m_symbolTableManager);
	initSymbolTable(symbolTable);
	return symbolTable;
}

void SymbolTableManager::Factory::initSymbolTable(AbstractSymbolTable* symbolTable) const {
	symbolTable->setMapper(m_symbolTableMapper);
	if (m_markAsNew)
		m_symbolTableManager->getProject()->getTransaction()->markAsNew(symbolTable);
}

SymbolTableManager::SymbolTableManager(Project* module)
	: AbstractItemManager(module)
{
	m_symbolTableMapper = new DB::SymbolTableMapper(this);
}

void SymbolTableManager::loadSymTables() const
{
	m_symbolTableMapper->loadAll();
}

SymbolTableManager::Factory SymbolTableManager::getFactory(bool markAsNew) {
	return Factory(this, m_symbolTableMapper, markAsNew);
}

AbstractSymbolTable* SymbolTableManager::findSymbolTableById(DB::Id id) {
	return dynamic_cast<AbstractSymbolTable*>(find(id));
}