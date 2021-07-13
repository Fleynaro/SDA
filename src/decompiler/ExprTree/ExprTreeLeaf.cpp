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

std::string FloatNanLeaf::printDebug() {
	return m_updateDebugInfo = ("NaN");
}

SymbolLeaf::SymbolLeaf(Symbol::Symbol* symbol)
	: m_symbol(symbol)
{}

int SymbolLeaf::getSize() {
	return m_symbol->getSize();
}

HS SymbolLeaf::getHash() {
	return m_symbol->getHash();
}

std::list<PCode::Instruction*> SymbolLeaf::getInstructionsRelatedTo() {
	if (auto symbolRelToInstr = dynamic_cast<IRelatedToInstruction*>(m_symbol))
		return symbolRelToInstr->getInstructionsRelatedTo();
	return {};
}

INode* SymbolLeaf::clone(NodeCloneContext* ctx) {
	return new SymbolLeaf(m_symbol->clone(ctx));
}

std::string SymbolLeaf::printDebug() {
	return m_updateDebugInfo = m_symbol->printDebug();
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
		(float&)m_value = static_cast<float>(value);
	else (double&)m_value = value;
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

std::string NumberLeaf::printDebug() {
	return m_updateDebugInfo = ("0x" + Helper::String::NumberToHex(m_value) + "{" + (std::to_string(static_cast<int>(m_value))) + "}");
}
