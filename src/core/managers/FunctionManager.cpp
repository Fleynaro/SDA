#include "FunctionManager.h"
#include "TypeManager.h"
#include "SymbolManager.h"
#include "SymbolTableManager.h"
#include <database/Mappers/FunctionMapper.h>
#include <ghidra_sync/Mappers/GhidraFunctionMapper.h>

using namespace CE;

FunctionManager::FunctionManager(Project* module)
	: AbstractItemManager(module)
{
	m_funcMapper = new DB::FunctionMapper(this);
	m_ghidraFunctionMapper = new Ghidra::FunctionMapper(this, getProject()->getTypeManager()->m_ghidraDataTypeMapper);
}

FunctionManager::~FunctionManager() {
	delete m_ghidraFunctionMapper;
}

FunctionManager::Factory FunctionManager::getFactory(bool markAsNew) {
	return Factory(this, m_ghidraFunctionMapper, m_funcMapper, markAsNew);
}

void FunctionManager::loadFunctions() const
{
	m_funcMapper->loadAll();
}

void FunctionManager::loadFunctionsFrom(ghidra::packet::SDataFullSyncPacket* dataPacket) const
{
	m_ghidraFunctionMapper->load(dataPacket);
}

Function* FunctionManager::findFunctionById(DB::Id id) {
	return dynamic_cast<Function*>(find(id));
}

Function* FunctionManager::findFunctionByGhidraId(Ghidra::Id id)
{
	Iterator it(this);
	while (it.hasNext()) {
		auto function = it.next();
		if (function->getGhidraId() == id) {
			return function;
		}
	}
	return nullptr;
}

Function* FunctionManager::Factory::createFunction(Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec, Symbol::SymbolTable* stackSymbolTable) const
{
	auto func = new Function(m_functionManager, functionSymbol, imageDec, stackSymbolTable);
	func->setMapper(m_funcMapper);
	func->setGhidraMapper(m_ghidraFunctionMapper);
	if(m_markAsNew)
		m_functionManager->getProject()->getTransaction()->markAsNew(func);
	return func;
}

Function* FunctionManager::Factory::createFunction(Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec) const
{
	const auto factory = m_functionManager->getProject()->getSymTableManager()->getFactory();
	const auto stackSymbolTable = factory.createSymbolTable(Symbol::SymbolTable::STACK_SPACE);
	return createFunction(functionSymbol, imageDec, stackSymbolTable);
}

Function* FunctionManager::Factory::createFunction(int64_t offset, DataType::IFunctionSignature* funcSignature, ImageDecorator* imageDec, const std::string& name, const std::string& comment) {
	const auto factory = m_functionManager->getProject()->getSymbolManager()->getFactory();
	const auto functionSymbol = factory.createFunctionSymbol(offset, funcSignature, name, comment);
	imageDec->getGlobalSymbolTable()->addSymbol(functionSymbol, offset);
	return createFunction(functionSymbol, imageDec);
}
