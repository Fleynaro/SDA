#include "ExprTreeSdaLeaf.h"
#include <datatypes/SystemType.h>

using namespace CE;
using namespace Decompiler;
using namespace ExprTree;

SdaNumberLeaf::SdaNumberLeaf(uint64_t value, DataTypePtr calcDataType)
	: m_value(value), m_calcDataType(calcDataType)
{}

uint64_t SdaNumberLeaf::getValue() {
	return m_value;
}

void SdaNumberLeaf::setValue(uint64_t value) {
	m_value = value;
}

int SdaNumberLeaf::getSize() {
	return getDataType()->getSize();
}

DataTypePtr SdaNumberLeaf::getSrcDataType() {
	return m_calcDataType;
}

void SdaNumberLeaf::setDataType(DataTypePtr dataType) {
	m_calcDataType = dataType;
}

ISdaNode* SdaNumberLeaf::cloneSdaNode(NodeCloneContext* ctx) {
	return new SdaNumberLeaf(m_value, m_calcDataType);
}

std::string SdaNumberLeaf::printSdaDebug() {
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

SdaSymbolLeaf::SdaSymbolLeaf(CE::Symbol::ISymbol* sdaSymbol, Symbol::Symbol* decSymbol)
	: m_sdaSymbol(sdaSymbol), m_decSymbol(decSymbol)
{}

Decompiler::Symbol::Symbol* SdaSymbolLeaf::getDecSymbol() const
{
	return m_decSymbol;
}

CE::Symbol::ISymbol* SdaSymbolLeaf::getSdaSymbol() const
{
	return m_sdaSymbol;
}

int SdaSymbolLeaf::getSize() {
	return getDataType()->getSize();
}

HS SdaSymbolLeaf::getHash() {
	return m_decSymbol->getHash();
}

ISdaNode* SdaSymbolLeaf::cloneSdaNode(NodeCloneContext* ctx) {
	return new SdaSymbolLeaf(m_sdaSymbol, m_decSymbol);
}

bool SdaSymbolLeaf::isFloatingPoint() {
	return false;
}

DataTypePtr SdaSymbolLeaf::getSrcDataType() {
	return m_sdaSymbol->getDataType();
}

void SdaSymbolLeaf::setDataType(DataTypePtr dataType) {
	if (m_sdaSymbol->isAutoSymbol()) {
		m_sdaSymbol->setDataType(dataType);
	}
}

std::string SdaSymbolLeaf::printSdaDebug() {
	return m_sdaSymbol->getName();
}

SdaMemSymbolLeaf::SdaMemSymbolLeaf(CE::Symbol::IMemorySymbol* sdaSymbol, Symbol::Symbol* decSymbol, int64_t offset, bool isAddrGetting)
	: SdaSymbolLeaf(sdaSymbol, decSymbol), m_offset(offset), m_isAddrGetting(isAddrGetting)
{}

CE::Symbol::IMemorySymbol* SdaMemSymbolLeaf::getSdaSymbol() const
{
	return dynamic_cast<CE::Symbol::IMemorySymbol*>(m_sdaSymbol);
}

DataTypePtr SdaMemSymbolLeaf::getSrcDataType() {
	if (m_isAddrGetting) {
		return MakePointer(SdaSymbolLeaf::getSrcDataType());
	}
	return SdaSymbolLeaf::getSrcDataType();
}

HS SdaMemSymbolLeaf::getHash() {
	return SdaSymbolLeaf::getHash() << m_offset;
}

ISdaNode* SdaMemSymbolLeaf::cloneSdaNode(NodeCloneContext* ctx) {
	return new SdaMemSymbolLeaf(getSdaSymbol(), m_decSymbol, m_offset, m_isAddrGetting);
}

bool SdaMemSymbolLeaf::isAddrGetting() {
	return m_isAddrGetting;
}

void SdaMemSymbolLeaf::setAddrGetting(bool toggle) {
	m_isAddrGetting = toggle;
}

void SdaMemSymbolLeaf::getLocation(MemLocation& location) {
	location.m_type = (getSdaSymbol()->getType() == CE::Symbol::LOCAL_STACK_VAR ? MemLocation::STACK : MemLocation::GLOBAL);
	location.m_offset = m_offset;
	location.m_valueSize = m_sdaSymbol->getDataType()->getSize();
}
