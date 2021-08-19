#include "ExprTreeLeaf.h"

using namespace CE::Decompiler;
using namespace ExprTree;

FloatNanLeaf::FloatNanLeaf()
{}

int FloatNanLeaf::getSize() {
	return 8;
}

HS FloatNanLeaf::getHash() {
	return HS();
}

INode* FloatNanLeaf::clone(NodeCloneContext* ctx) {
	return new FloatNanLeaf();
}

bool FloatNanLeaf::isFloatingPoint() {
	return true;
}

SymbolLeaf::SymbolLeaf(Symbol::Symbol* symbol)
	: m_symbol(symbol)
{
	m_symbol->m_parentsCount++;
}

SymbolLeaf::~SymbolLeaf() {
	m_symbol->m_parentsCount--;
}

int SymbolLeaf::getSize() {
	return m_symbol->getSize();
}

HS SymbolLeaf::getHash() {
	return m_symbol->getHash();
}

std::list<PCode::Instruction*> SymbolLeaf::getInstructionsRelatedTo() {
	return {};
}

INode* SymbolLeaf::clone(NodeCloneContext* ctx) {
	return new SymbolLeaf(m_symbol->clone(ctx));
}

StoragePath SymbolLeaf::getStoragePath() {
	StoragePath path;
	path.m_symbol = m_symbol;
	return path;
}

HS INumberLeaf::getHash() {
	return HS() << getValue();
}

NumberLeaf::NumberLeaf(uint64_t value, int size)
	: m_value(value& BitMask64(size).getValue()), m_size(size)
{}

NumberLeaf::NumberLeaf(double value, int size)
	: m_size(size)
{
	if (m_size == 4)
		reinterpret_cast<float&>(m_value) = static_cast<float>(value);
	else reinterpret_cast<double&>(m_value) = value;
}

uint64_t NumberLeaf::getValue() {
	return m_value;
}

void NumberLeaf::setValue(uint64_t value) {
	m_value = value;
}

int NumberLeaf::getSize() {
	return m_size;
}

INode* NumberLeaf::clone(NodeCloneContext* ctx) {
	return new NumberLeaf(m_value, m_size);
}