#include "ExprTreeSdaLeaf.h"

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

SdaSymbolLeaf::SdaSymbolLeaf(SymbolLeaf* m_symbolLeaf, CE::Symbol::ISymbol* sdaSymbol, bool isAddrGetting)
	: m_symbolLeaf(m_symbolLeaf), m_sdaSymbol(sdaSymbol), m_isAddrGetting(isAddrGetting)
{}

void SdaSymbolLeaf::replaceNode(INode* node, INode* newNode) {
	if (node == m_symbolLeaf) {
		m_symbolLeaf = dynamic_cast<SymbolLeaf*>(newNode);
	}
}

std::list<INode*> SdaSymbolLeaf::getNodesList() {
	return {m_symbolLeaf};
}

Decompiler::Symbol::Symbol* SdaSymbolLeaf::getDecSymbol() const {
	return m_symbolLeaf->m_symbol;
}

CE::Symbol::ISymbol* SdaSymbolLeaf::getSdaSymbol() const {
	return m_sdaSymbol;
}

int SdaSymbolLeaf::getSize() {
	return getDataType()->getSize();
}

HS SdaSymbolLeaf::getHash() {
	const auto storage = m_sdaSymbol->getStorage();
	return m_symbolLeaf->getHash()
		<< storage.getOffset();
}

ISdaNode* SdaSymbolLeaf::cloneSdaNode(NodeCloneContext* ctx) {
	return new SdaSymbolLeaf(m_symbolLeaf, m_sdaSymbol, m_isAddrGetting);
}

bool SdaSymbolLeaf::isFloatingPoint() {
	return false;
}

DataTypePtr SdaSymbolLeaf::getSrcDataType() {
	if (m_isAddrGetting) {
		return MakePointer(m_sdaSymbol->getDataType());
	}
	return m_sdaSymbol->getDataType();
}

void SdaSymbolLeaf::setDataType(DataTypePtr dataType) {
	if (m_sdaSymbol->isAutoSymbol()) {
		if (m_isAddrGetting && dataType->isPointer()) {
			m_sdaSymbol->setDataType(DereferencePointer(dataType));
		}
		else {
			m_sdaSymbol->setDataType(dataType);
		}
	}
}

bool SdaSymbolLeaf::isAddrGetting() {
	return m_isAddrGetting;
}

void SdaSymbolLeaf::setAddrGetting(bool toggle) {
	m_isAddrGetting = toggle;
}

bool SdaSymbolLeaf::getLocation(MemLocation& location) {
	const auto storage = m_sdaSymbol->getStorage();
	if (storage.getType() == Storage::STORAGE_REGISTER) {
		location.m_type = MemLocation::IMPLICIT;
		location.m_baseAddrHash = getHash();
		return true;
	}
	location.m_type = storage.getType() == Storage::STORAGE_STACK ? MemLocation::STACK : MemLocation::GLOBAL;
	location.m_offset = storage.getOffset();
	location.m_valueSize = m_sdaSymbol->getDataType()->getSize();
	return true;
}

StoragePath SdaSymbolLeaf::getStoragePath() {
	auto path = m_symbolLeaf->getStoragePath();
	const auto storage = m_sdaSymbol->getStorage();
	if (storage.getType() == Storage::STORAGE_STACK || storage.getType() == Storage::STORAGE_GLOBAL) {
		path.m_offsets.push_back(storage.getOffset());
	}
	return path;
}