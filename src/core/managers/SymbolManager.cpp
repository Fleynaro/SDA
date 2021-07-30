#include "SymbolManager.h"
#include <database/Mappers/SymbolMapper.h>

using namespace CE;
using namespace Symbol;

SymbolManager::SymbolManager(Project* module)
	: AbstractItemManager(module)
{
	m_symbolMapper = new DB::SymbolMapper(this);

	auto defType = getProject()->getTypeManager()->findTypeById(DataType::SystemType::Byte);
	m_defGlobVarSymbol = getFactory(false).createGlobalVarSymbol(0, GetUnit(defType), "def_symbol");
}

void SymbolManager::loadSymbols() const
{
	m_symbolMapper->loadAll();
}

AbstractSymbol* SymbolManager::findSymbolById(DB::Id id) {
	return dynamic_cast<AbstractSymbol*>(find(id));
}

GlobalVarSymbol* SymbolManager::getDefGlobalVarSymbol() {
	return m_defGlobVarSymbol;
}

SymbolManager::Factory SymbolManager::getFactory(bool markAsNew) {
	return Factory(this, m_symbolMapper, markAsNew);
}

FuncParameterSymbol* SymbolManager::Factory::createFuncParameterSymbol(int paramIdx, DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new FuncParameterSymbol(m_symbolManager, type, name, comment);
	symbol->m_paramIdx = paramIdx;
	bind(symbol);
	return symbol;
}

StructFieldSymbol* SymbolManager::Factory::createStructFieldSymbol(int absBitOffset, int bitSize, DataType::IStructure* structure, DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new StructFieldSymbol(m_symbolManager, structure, type, absBitOffset, bitSize, name, comment);
	bind(symbol);
	return symbol;
}

FunctionSymbol* SymbolManager::Factory::createFunctionSymbol(int64_t offset, DataType::IFunctionSignature* funcSignature, const std::string& name, const std::string& comment) const
{
	const auto symbol = new FunctionSymbol(m_symbolManager, offset, funcSignature, name, comment);
	bind(symbol);
	return symbol;
}

LocalInstrVarSymbol* SymbolManager::Factory::createLocalInstrVarSymbol(DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new LocalInstrVarSymbol(m_symbolManager, type, name, comment);
	bind(symbol);
	return symbol;
}

GlobalVarSymbol* SymbolManager::Factory::createGlobalVarSymbol(int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new GlobalVarSymbol(m_symbolManager, offset, type, name, comment);
	bind(symbol);
	return symbol;
}

LocalStackVarSymbol* SymbolManager::Factory::createLocalStackVarSymbol(int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new LocalStackVarSymbol(m_symbolManager, offset, type, name, comment);
	bind(symbol);
	return symbol;
}

void SymbolManager::Factory::bind(AbstractSymbol* symbol) const
{
	symbol->setMapper(m_symbolMapper);
	if (m_markAsNew)
		m_symbolManager->getProject()->getTransaction()->markAsNew(symbol);
}
