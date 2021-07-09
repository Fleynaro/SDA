#include "ExprTreeSdaLeaf.h"
#include <datatypes/SystemType.h>

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::SdaNumberLeaf::SdaNumberLeaf(uint64_t value, DataTypePtr calcDataType)
	: m_value(value), m_calcDataType(calcDataType)
{}

uint64_t CE::Decompiler::ExprTree::SdaNumberLeaf::getValue() {
	return m_value;
}

void CE::Decompiler::ExprTree::SdaNumberLeaf::setValue(uint64_t value) {
	m_value = value;
}

int CE::Decompiler::ExprTree::SdaNumberLeaf::getSize() {
	return getDataType()->getSize();
}

DataTypePtr CE::Decompiler::ExprTree::SdaNumberLeaf::getSrcDataType() {
	return m_calcDataType;
}

void CE::Decompiler::ExprTree::SdaNumberLeaf::setDataType(DataTypePtr dataType) {
	m_calcDataType = dataType;
}

ISdaNode* CE::Decompiler::ExprTree::SdaNumberLeaf::cloneSdaNode(NodeCloneContext* ctx) {
	return new SdaNumberLeaf(m_value, m_calcDataType);
}

std::string CE::Decompiler::ExprTree::SdaNumberLeaf::printSdaDebug() {
	if (getSrcDataType()->isFloatingPoint()) {
		if (getSrcDataType()->getSize() == 4)
			return m_updateDebugInfo = std::to_string((float&)m_value);
		else return m_updateDebugInfo = std::to_string((double&)m_value);
	}
	if (auto sysType = dynamic_cast<DataType::SystemType*>(getSrcDataType()->getBaseType())) {
		if (sysType->isSigned()) {
			const auto size = getSrcDataType()->getSize();
			if (size <= 4)
				return m_updateDebugInfo = std::to_string(static_cast<int32_t>(m_value));
			else
				return m_updateDebugInfo = std::to_string(static_cast<int64_t>(m_value));
		}
	}
	return "0x" + Helper::String::NumberToHex(m_value);
}

CE::Decompiler::ExprTree::SdaSymbolLeaf::SdaSymbolLeaf(CE::Symbol::ISymbol* sdaSymbol, Symbol::Symbol* decSymbol)
	: m_sdaSymbol(sdaSymbol), m_decSymbol(decSymbol)
{}

Decompiler::Symbol::Symbol* CE::Decompiler::ExprTree::SdaSymbolLeaf::getDecSymbol() const
{
	return m_decSymbol;
}

CE::Symbol::ISymbol* CE::Decompiler::ExprTree::SdaSymbolLeaf::getSdaSymbol() const
{
	return m_sdaSymbol;
}

int CE::Decompiler::ExprTree::SdaSymbolLeaf::getSize() {
	return getDataType()->getSize();
}

HS CE::Decompiler::ExprTree::SdaSymbolLeaf::getHash() {
	return m_decSymbol->getHash();
}

ISdaNode* CE::Decompiler::ExprTree::SdaSymbolLeaf::cloneSdaNode(NodeCloneContext* ctx) {
	return new SdaSymbolLeaf(m_sdaSymbol, m_decSymbol);
}

bool CE::Decompiler::ExprTree::SdaSymbolLeaf::isFloatingPoint() {
	return false;
}

DataTypePtr CE::Decompiler::ExprTree::SdaSymbolLeaf::getSrcDataType() {
	return m_sdaSymbol->getDataType();
}

void CE::Decompiler::ExprTree::SdaSymbolLeaf::setDataType(DataTypePtr dataType) {
	if (m_sdaSymbol->isAutoSymbol()) {
		m_sdaSymbol->setDataType(dataType);
	}
}

std::string CE::Decompiler::ExprTree::SdaSymbolLeaf::printSdaDebug() {
	return m_sdaSymbol->getName();
}

CE::Decompiler::ExprTree::SdaMemSymbolLeaf::SdaMemSymbolLeaf(CE::Symbol::IMemorySymbol* sdaSymbol, Symbol::Symbol* decSymbol, int64_t offset, bool isAddrGetting)
	: SdaSymbolLeaf(sdaSymbol, decSymbol), m_offset(offset), m_isAddrGetting(isAddrGetting)
{}

CE::Symbol::IMemorySymbol* CE::Decompiler::ExprTree::SdaMemSymbolLeaf::getSdaSymbol() const
{
	return dynamic_cast<CE::Symbol::IMemorySymbol*>(m_sdaSymbol);
}

DataTypePtr CE::Decompiler::ExprTree::SdaMemSymbolLeaf::getSrcDataType() {
	if (m_isAddrGetting) {
		return MakePointer(SdaSymbolLeaf::getSrcDataType());
	}
	return SdaSymbolLeaf::getSrcDataType();
}

HS CE::Decompiler::ExprTree::SdaMemSymbolLeaf::getHash() {
	return SdaSymbolLeaf::getHash() << m_offset;
}

ISdaNode* CE::Decompiler::ExprTree::SdaMemSymbolLeaf::cloneSdaNode(NodeCloneContext* ctx) {
	return new SdaMemSymbolLeaf(getSdaSymbol(), m_decSymbol, m_offset, m_isAddrGetting);
}

bool CE::Decompiler::ExprTree::SdaMemSymbolLeaf::isAddrGetting() {
	return m_isAddrGetting;
}

void CE::Decompiler::ExprTree::SdaMemSymbolLeaf::setAddrGetting(bool toggle) {
	m_isAddrGetting = toggle;
}

void CE::Decompiler::ExprTree::SdaMemSymbolLeaf::getLocation(MemLocation& location) {
	location.m_type = (getSdaSymbol()->getType() == CE::Symbol::LOCAL_STACK_VAR ? MemLocation::STACK : MemLocation::GLOBAL);
	location.m_offset = m_offset;
	location.m_valueSize = m_sdaSymbol->getDataType()->getSize();
}
