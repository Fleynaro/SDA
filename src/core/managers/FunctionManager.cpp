#include "FunctionManager.h"
#include "TypeManager.h"
#include "SymbolManager.h"
#include "SymbolTableManager.h"
#include "ImageDecorator.h"
#include <database/Mappers/FunctionMapper.h>

using namespace CE;

FunctionManager::FunctionManager(Project* module)
	: AbstractItemManager(module)
{
	m_funcMapper = new DB::FunctionMapper(this);
}

FunctionManager::~FunctionManager() {
}

FunctionManager::Factory FunctionManager::getFactory(bool markAsNew) {
	return Factory(this, m_funcMapper, markAsNew);
}

void FunctionManager::loadFunctions() const
{
	m_funcMapper->loadAll();
}

Function* FunctionManager::findFunctionById(DB::Id id) {
	return dynamic_cast<Function*>(find(id));
}

Function* FunctionManager::Factory::createFunction(Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec, Symbol::StackSymbolTable* stackSymbolTable) const
{
	auto func = new Function(m_functionManager, functionSymbol, imageDec, stackSymbolTable);
	func->setMapper(m_funcMapper);
	if(m_markAsNew)
		m_functionManager->getProject()->getTransaction()->markAsNew(func);
	return func;
}

Function* FunctionManager::Factory::createFunction(Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec) const
{
	const auto factory = m_functionManager->getProject()->getSymTableManager()->getFactory();
	const auto stackSymbolTable = factory.createStackSymbolTable();
	return createFunction(functionSymbol, imageDec, stackSymbolTable);
}

Function* FunctionManager::Factory::createFunction(int64_t offset, DataType::IFunctionSignature* funcSignature, ImageDecorator* imageDec, const std::string& name, const std::string& comment) {
	const auto factory = m_functionManager->getProject()->getSymbolManager()->getFactory();
	const auto functionSymbol = factory.createFunctionSymbol(offset, funcSignature, name, comment);
	imageDec->getGlobalSymbolTable()->addSymbol(functionSymbol, offset);
	return createFunction(functionSymbol, imageDec);
}
