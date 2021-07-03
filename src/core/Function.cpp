#include "Function.h"
#include <managers/FunctionManager.h>
#include <decompiler/Graph/DecPCodeGraph.h>

using namespace CE;

CE::Function::Function(FunctionManager* manager, Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec, Symbol::SymbolTable* stackSymbolTable)
	: m_manager(manager), m_functionSymbol(functionSymbol), m_imageDec(imageDec), m_stackSymbolTable(stackSymbolTable)
{
	functionSymbol->setFunction(this);
}

SymbolContext CE::Function::getSymbolContext() const
{
	SymbolContext symbolCtx;
	symbolCtx.m_signature = getSignature();
	symbolCtx.m_globalSymbolTable = m_imageDec->getGlobalSymbolTable();
	symbolCtx.m_funcBodySymbolTable = m_imageDec->getFuncBodySymbolTable();
	symbolCtx.m_stackSymbolTable = m_stackSymbolTable;
	return symbolCtx;
}

Symbol::FunctionSymbol* Function::getFunctionSymbol() const
{
	return m_functionSymbol;
}

ImageDecorator* CE::Function::getImage() const
{
	return m_imageDec;
}

Decompiler::FunctionPCodeGraph* Function::getFuncGraph() const
{
	return m_imageDec->getPCodeGraph()->getFuncGraphAt(getOffset());
}

const std::string Function::getName() {
	return m_functionSymbol->getName();
}

const std::string Function::getComment() {
	return m_functionSymbol->getComment();
}

void Function::setName(const std::string& name) {
	m_functionSymbol->setName(name);
}

void Function::setComment(const std::string& comment) {
	m_functionSymbol->setComment(comment);
}

DataType::IFunctionSignature* Function::getSignature() const
{
	return m_functionSymbol->getSignature();
}

int64_t CE::Function::getOffset() const
{
	return m_functionSymbol->getOffset();
}

Symbol::SymbolTable* Function::getStackSymbolTable() const
{
	return m_stackSymbolTable;
}

Ghidra::Id Function::getGhidraId()
{
	return getOffset();
}

FunctionManager* Function::getManager() const
{
	return m_manager;
}
