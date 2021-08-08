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

SdaSymbolLeaf::SdaSymbolLeaf(SymbolLeaf* m_symbolLeaf, CE::Symbol::ISymbol* sdaSymbol)
	: m_symbolLeaf(m_symbolLeaf), m_sdaSymbol(sdaSymbol)
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
	return m_symbolLeaf->getHash();
}

ISdaNode* SdaSymbolLeaf::cloneSdaNode(NodeCloneContext* ctx) {
	return new SdaSymbolLeaf(m_symbolLeaf, m_sdaSymbol);
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

StoragePath SdaSymbolLeaf::getStoragePath() {
	return m_symbolLeaf->getStoragePath();
}

SdaMemSymbolLeaf::SdaMemSymbolLeaf(SymbolLeaf* symbolLeaf, CE::Symbol::AbstractMemorySymbol* sdaMemSymbol, bool isAddrGetting)
	: SdaSymbolLeaf(symbolLeaf, sdaMemSymbol), m_sdaMemSymbol(sdaMemSymbol), m_isAddrGetting(isAddrGetting)
{}

CE::Symbol::AbstractMemorySymbol* SdaMemSymbolLeaf::getSdaMemSymbol() const {
	return m_sdaMemSymbol;
}

DataTypePtr SdaMemSymbolLeaf::getSrcDataType() {
	if (m_isAddrGetting) {
		return MakePointer(SdaSymbolLeaf::getSrcDataType());
	}
	return SdaSymbolLeaf::getSrcDataType();
}

HS SdaMemSymbolLeaf::getHash() {
	return SdaSymbolLeaf::getHash() << m_sdaMemSymbol->getOffset();
}

ISdaNode* SdaMemSymbolLeaf::cloneSdaNode(NodeCloneContext* ctx) {
	return new SdaMemSymbolLeaf(m_symbolLeaf, m_sdaMemSymbol, m_isAddrGetting);
}

bool SdaMemSymbolLeaf::isAddrGetting() {
	return m_isAddrGetting;
}

void SdaMemSymbolLeaf::setAddrGetting(bool toggle) {
	m_isAddrGetting = toggle;
}

void SdaMemSymbolLeaf::getLocation(MemLocation& location) {
	location.m_type = m_sdaMemSymbol->getType() == CE::Symbol::LOCAL_STACK_VAR ? MemLocation::STACK : MemLocation::GLOBAL;
	location.m_offset = m_sdaMemSymbol->getOffset();
	location.m_valueSize = m_sdaSymbol->getDataType()->getSize();
}

StoragePath SdaMemSymbolLeaf::getStoragePath() {
	StoragePath path;
	if(m_sdaMemSymbol->getType() == CE::Symbol::LOCAL_STACK_VAR) {
		path.m_register = PCode::Register(ZYDIS_REGISTER_RSP, 0, BitMask64(0x8), PCode::Register::Type::StackPointer);
	} else {
		path.m_register = PCode::Register(ZYDIS_REGISTER_RIP, 0, BitMask64(0x8), PCode::Register::Type::InstructionPointer);
	}
	path.m_offsets.push_back(m_sdaMemSymbol->getOffset());
	return path;
}
