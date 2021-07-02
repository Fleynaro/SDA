#include "ExprTreeLeaf.h"

using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::FloatNanLeaf::FloatNanLeaf()
{}

int CE::Decompiler::ExprTree::FloatNanLeaf::getSize() {
	return 8;
}

HS CE::Decompiler::ExprTree::FloatNanLeaf::getHash() {
	return HS();
}

INode* CE::Decompiler::ExprTree::FloatNanLeaf::clone(NodeCloneContext* ctx) {
	return new FloatNanLeaf();
}

bool CE::Decompiler::ExprTree::FloatNanLeaf::isFloatingPoint() {
	return true;
}

std::string CE::Decompiler::ExprTree::FloatNanLeaf::printDebug() {
	return m_updateDebugInfo = ("NaN");
}

CE::Decompiler::ExprTree::SymbolLeaf::SymbolLeaf(Symbol::Symbol* symbol)
	: m_symbol(symbol)
{}

int CE::Decompiler::ExprTree::SymbolLeaf::getSize() {
	return m_symbol->getSize();
}

HS CE::Decompiler::ExprTree::SymbolLeaf::getHash() {
	return m_symbol->getHash();
}

std::list<PCode::Instruction*> CE::Decompiler::ExprTree::SymbolLeaf::getInstructionsRelatedTo() {
	if (auto symbolRelToInstr = dynamic_cast<PCode::IRelatedToInstruction*>(m_symbol))
		return symbolRelToInstr->getInstructionsRelatedTo();
	return {};
}

INode* CE::Decompiler::ExprTree::SymbolLeaf::clone(NodeCloneContext* ctx) {
	return new SymbolLeaf(m_symbol->clone(ctx));
}

std::string CE::Decompiler::ExprTree::SymbolLeaf::printDebug() {
	return m_updateDebugInfo = m_symbol->printDebug();
}

HS CE::Decompiler::ExprTree::INumberLeaf::getHash() {
	return HS() << getValue();
}

CE::Decompiler::ExprTree::NumberLeaf::NumberLeaf(uint64_t value, int size)
	: m_value(value& BitMask64(size).getValue()), m_size(size)
{}

CE::Decompiler::ExprTree::NumberLeaf::NumberLeaf(double value, int size)
	: m_size(size)
{
	if (m_size == 4)
		(float&)m_value = (float)value;
	else (double&)m_value = value;
}

uint64_t CE::Decompiler::ExprTree::NumberLeaf::getValue() {
	return m_value;
}

void CE::Decompiler::ExprTree::NumberLeaf::setValue(uint64_t value) {
	m_value = value;
}

int CE::Decompiler::ExprTree::NumberLeaf::getSize() {
	return m_size;
}

INode* CE::Decompiler::ExprTree::NumberLeaf::clone(NodeCloneContext* ctx) {
	return new NumberLeaf(m_value, m_size);
}

std::string CE::Decompiler::ExprTree::NumberLeaf::printDebug() {
	return m_updateDebugInfo = ("0x" + Helper::String::NumberToHex(m_value) + "{" + (std::to_string((int)m_value)) + "}");
}
