#include "SymbolManager.h"
#include <database/Mappers/SymbolMapper.h>

using namespace CE;
using namespace CE::Symbol;

SymbolManager::SymbolManager(Project* module)
	: AbstractItemManager(module)
{
	m_symbolMapper = new DB::SymbolMapper(this);

	auto defType = getProject()->getTypeManager()->findTypeById(CE::DataType::SystemType::Byte);
	m_defGlobVarSymbol = getFactory(false).createGlobalVarSymbol(0, DataType::GetUnit(defType), "def_symbol");
}

void SymbolManager::loadSymbols() const
{
	m_symbolMapper->loadAll();
}

Symbol::AbstractSymbol* CE::SymbolManager::findSymbolById(DB::Id id) {
	return dynamic_cast<Symbol::AbstractSymbol*>(find(id));
}

Symbol::GlobalVarSymbol* SymbolManager::getDefGlobalVarSymbol() {
	return m_defGlobVarSymbol;
}

SymbolManager::Factory SymbolManager::getFactory(bool markAsNew) {
	return Factory(this, m_symbolMapper, markAsNew);
}

Symbol::FuncParameterSymbol* SymbolManager::Factory::createFuncParameterSymbol(int paramIdx, DataType::IFunctionSignature* signature, DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new Symbol::FuncParameterSymbol(m_symbolManager, paramIdx, signature, type, name, comment);
	bind(symbol);
	return symbol;
}

Symbol::StructFieldSymbol* SymbolManager::Factory::createStructFieldSymbol(int absBitOffset, int bitSize, DataType::IStructure* structure, DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new Symbol::StructFieldSymbol(m_symbolManager, structure, type, absBitOffset, bitSize, name, comment);
	bind(symbol);
	return symbol;
}

Symbol::FunctionSymbol* SymbolManager::Factory::createFunctionSymbol(int64_t offset, DataType::IFunctionSignature* funcSignature, const std::string& name, const std::string& comment) const
{
	const auto symbol = new Symbol::FunctionSymbol(m_symbolManager, offset, funcSignature, name, comment);
	bind(symbol);
	return symbol;
}

Symbol::LocalInstrVarSymbol* SymbolManager::Factory::createLocalInstrVarSymbol(DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new Symbol::LocalInstrVarSymbol(m_symbolManager, type, name, comment);
	bind(symbol);
	return symbol;
}

Symbol::GlobalVarSymbol* SymbolManager::Factory::createGlobalVarSymbol(int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new Symbol::GlobalVarSymbol(m_symbolManager, offset, type, name, comment);
	bind(symbol);
	return symbol;
}

Symbol::LocalStackVarSymbol* SymbolManager::Factory::createLocalStackVarSymbol(int64_t offset, DataTypePtr type, const std::string& name, const std::string& comment) const
{
	const auto symbol = new Symbol::LocalStackVarSymbol(m_symbolManager, offset, type, name, comment);
	bind(symbol);
	return symbol;
}

void SymbolManager::Factory::bind(Symbol::AbstractSymbol* symbol) const
{
	symbol->setMapper(m_symbolMapper);
	if (m_markAsNew)
		m_symbolManager->getProject()->getTransaction()->markAsNew(symbol);
}
