#include "ExprTreeSdaGoar.h"

using namespace CE;
using namespace Decompiler;
using namespace ExprTree;

// for players[0].pos.x the base is "players" array

GoarTopNode::GoarTopNode(ISdaNode* base, int64_t bitOffset, bool isAddrGetting)
	: GoarNode(base), m_bitOffset(bitOffset), m_isAddrGetting(isAddrGetting)
{}

bool GoarTopNode::isAddrGetting() {
	return m_isAddrGetting;
}

void GoarTopNode::setAddrGetting(bool toggle) {
	m_isAddrGetting = toggle;
}

void GoarTopNode::getLocation(MemLocation& location) {
	auto mainBase = getBaseNode(this);
	if (auto storedInMem = dynamic_cast<IMappedToMemory*>(mainBase)) {
		storedInMem->getLocation(location);
	}
	else {
		location.m_type = MemLocation::IMPLICIT;
		location.m_baseAddrHash = mainBase->getHash();
	}
	location.m_offset += m_bitOffset / 0x8;
	location.m_valueSize = m_base->getDataType()->getSize();
	gatherArrDims(m_base, location);
}

DataTypePtr GoarTopNode::getSrcDataType() {
	if (m_isAddrGetting) {
		return MakePointer(m_base->getDataType());
	}
	return m_base->getDataType();
}

HS GoarTopNode::getHash() {
	return GoarNode::getHash() << m_isAddrGetting;
}

ISdaNode* GoarTopNode::cloneSdaNode(NodeCloneContext* ctx) {
	return new GoarTopNode(dynamic_cast<ISdaNode*>(m_base->clone()), m_bitOffset, m_isAddrGetting);
}


// players[2][10] -> dims: 10, 2

void GoarTopNode::gatherArrDims(ISdaNode* node, MemLocation& location) {
	if (const auto goarNode = dynamic_cast<GoarNode*>(node)) {
		gatherArrDims(goarNode->m_base, location);
		if (auto goarArrayNode = dynamic_cast<GoarArrayNode*>(node)) {
			if (dynamic_cast<INumberLeaf*>(goarArrayNode->m_indexNode))
				return;
			const auto itemSize = goarArrayNode->getDataType()->getSize();
			const auto itemsMaxCount = (goarArrayNode->m_itemsMaxCount > 1 ? goarArrayNode->m_itemsMaxCount : -1);
			location.addArrayDim(itemSize, itemsMaxCount);
		}
	}
}

ISdaNode* GoarTopNode::getBaseNode(ISdaNode* node) {
	if (const auto goarNode = dynamic_cast<GoarNode*>(node)) {
		return getBaseNode(goarNode->m_base);
	}
	return node;
}

GoarNode::GoarNode(ISdaNode* base)
	: m_base(base)
{
	m_base->addParentNode(this);
}

GoarNode::~GoarNode() {
	m_base->removeBy(this);
}

void GoarNode::replaceNode(INode* node, INode* newNode) {
	const auto newSdaNode = dynamic_cast<ISdaNode*>(newNode);
	if (node == m_base) {
		m_base = newSdaNode;
	}
}

std::list<INode*> GoarNode::getNodesList() {
	return { m_base };
}

int GoarNode::getSize() {
	return getSrcDataType()->getSize();
}

HS GoarNode::getHash() {
	return m_base->getHash();
}

bool GoarNode::isFloatingPoint() {
	return false;
}

void GoarNode::setDataType(DataTypePtr dataType) {
}

GoarArrayNode::GoarArrayNode(ISdaNode* base, ISdaNode* indexNode, DataTypePtr dataType, int itemsMaxCount)
	: GoarNode(base), m_indexNode(indexNode), m_outDataType(dataType), m_itemsMaxCount(itemsMaxCount)
{
	m_indexNode->addParentNode(this);
}

GoarArrayNode::~GoarArrayNode() {
	m_indexNode->removeBy(this);
}

void GoarArrayNode::replaceNode(INode* node, INode* newNode) {
	GoarNode::replaceNode(node, newNode);
	const auto newSdaNode = dynamic_cast<ISdaNode*>(newNode);
	if (node == m_indexNode) {
		m_indexNode = newSdaNode;
	}
}

std::list<INode*> GoarArrayNode::getNodesList() {
	return { m_base, m_indexNode };
}

DataTypePtr GoarArrayNode::getSrcDataType() {
	return m_outDataType;
}

HS GoarArrayNode::getHash() {
	return GoarNode::getHash() << m_indexNode->getHash();
}

ISdaNode* GoarArrayNode::cloneSdaNode(NodeCloneContext* ctx) {
	return new GoarArrayNode(dynamic_cast<ISdaNode*>(m_base->clone()), dynamic_cast<ISdaNode*>(m_indexNode->clone(ctx)), CloneUnit(m_outDataType), m_itemsMaxCount);
}

GoarFieldNode::GoarFieldNode(ISdaNode* base, CE::Symbol::StructFieldSymbol* field)
	: GoarNode(base), m_field(field)
{}

DataTypePtr GoarFieldNode::getSrcDataType() {
	return m_field->getDataType();
}

ISdaNode* GoarFieldNode::cloneSdaNode(NodeCloneContext* ctx) {
	return new GoarFieldNode(dynamic_cast<ISdaNode*>(m_base->clone()), m_field);
}