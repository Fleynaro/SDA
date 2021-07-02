#include "ExprTreeSdaGoar.h"

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

// for players[0].pos.x the base is "players" array

CE::Decompiler::ExprTree::GoarTopNode::GoarTopNode(ISdaNode* base, int64_t bitOffset, bool isAddrGetting)
	: GoarNode(base), m_bitOffset(bitOffset), m_isAddrGetting(isAddrGetting)
{}

bool CE::Decompiler::ExprTree::GoarTopNode::isAddrGetting() {
	return m_isAddrGetting;
}

void CE::Decompiler::ExprTree::GoarTopNode::setAddrGetting(bool toggle) {
	m_isAddrGetting = toggle;
}

void CE::Decompiler::ExprTree::GoarTopNode::getLocation(MemLocation& location) {
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

DataTypePtr CE::Decompiler::ExprTree::GoarTopNode::getSrcDataType() {
	if (m_isAddrGetting) {
		return MakePointer(m_base->getDataType());
	}
	return m_base->getDataType();
}

HS CE::Decompiler::ExprTree::GoarTopNode::getHash() {
	return GoarNode::getHash() << m_isAddrGetting;
}

ISdaNode* CE::Decompiler::ExprTree::GoarTopNode::cloneSdaNode(NodeCloneContext* ctx) {
	return new GoarTopNode(dynamic_cast<ISdaNode*>(m_base->clone()), m_bitOffset, m_isAddrGetting);
}

std::string CE::Decompiler::ExprTree::GoarTopNode::printSdaDebug() {
	return m_base->printSdaDebug();
}


// players[2][10] -> dims: 10, 2

void CE::Decompiler::ExprTree::GoarTopNode::gatherArrDims(ISdaNode* node, MemLocation& location) {
	if (auto goarNode = dynamic_cast<GoarNode*>(node)) {
		gatherArrDims(goarNode->m_base, location);
		if (auto goarArrayNode = dynamic_cast<GoarArrayNode*>(node)) {
			if (dynamic_cast<INumberLeaf*>(goarArrayNode->m_indexNode))
				return;
			auto itemSize = goarArrayNode->getDataType()->getSize();
			auto itemsMaxCount = (goarArrayNode->m_itemsMaxCount > 1 ? goarArrayNode->m_itemsMaxCount : -1);
			location.addArrayDim(itemSize, itemsMaxCount);
		}
	}
}

ISdaNode* CE::Decompiler::ExprTree::GoarTopNode::getBaseNode(ISdaNode* node) {
	if (auto goarNode = dynamic_cast<GoarNode*>(node)) {
		return getBaseNode(goarNode->m_base);
	}
	return node;
}

CE::Decompiler::ExprTree::GoarNode::GoarNode(ISdaNode* base)
	: m_base(base)
{
	m_base->addParentNode(this);
}

CE::Decompiler::ExprTree::GoarNode::~GoarNode() {
	m_base->removeBy(this);
}

void CE::Decompiler::ExprTree::GoarNode::replaceNode(INode* node, INode* newNode) {
	auto newSdaNode = dynamic_cast<ISdaNode*>(newNode);
	if (node == m_base) {
		m_base = newSdaNode;
	}
}

std::list<ExprTree::INode*> CE::Decompiler::ExprTree::GoarNode::getNodesList() {
	return { m_base };
}

int CE::Decompiler::ExprTree::GoarNode::getSize() {
	return getSrcDataType()->getSize();
}

HS CE::Decompiler::ExprTree::GoarNode::getHash() {
	return m_base->getHash();
}

bool CE::Decompiler::ExprTree::GoarNode::isFloatingPoint() {
	return false;
}

void CE::Decompiler::ExprTree::GoarNode::setDataType(DataTypePtr dataType) {
}

CE::Decompiler::ExprTree::GoarArrayNode::GoarArrayNode(ISdaNode* base, ISdaNode* indexNode, DataTypePtr dataType, int itemsMaxCount)
	: GoarNode(base), m_indexNode(indexNode), m_outDataType(dataType), m_itemsMaxCount(itemsMaxCount)
{
	m_indexNode->addParentNode(this);
}

CE::Decompiler::ExprTree::GoarArrayNode::~GoarArrayNode() {
	m_indexNode->removeBy(this);
}

void CE::Decompiler::ExprTree::GoarArrayNode::replaceNode(INode* node, INode* newNode) {
	GoarNode::replaceNode(node, newNode);
	auto newSdaNode = dynamic_cast<ISdaNode*>(newNode);
	if (node == m_indexNode) {
		m_indexNode = newSdaNode;
	}
}

std::list<ExprTree::INode*> CE::Decompiler::ExprTree::GoarArrayNode::getNodesList() {
	return { m_base, m_indexNode };
}

DataTypePtr CE::Decompiler::ExprTree::GoarArrayNode::getSrcDataType() {
	return m_outDataType;
}

HS CE::Decompiler::ExprTree::GoarArrayNode::getHash() {
	return GoarNode::getHash() << m_indexNode->getHash();
}

ISdaNode* CE::Decompiler::ExprTree::GoarArrayNode::cloneSdaNode(NodeCloneContext* ctx) {
	return new GoarArrayNode(dynamic_cast<ISdaNode*>(m_base->clone()), dynamic_cast<ISdaNode*>(m_indexNode->clone(ctx)), CloneUnit(m_outDataType), m_itemsMaxCount);
}

std::string CE::Decompiler::ExprTree::GoarArrayNode::printSdaDebug() {
	auto str = m_base->printSdaDebug();
	str = str + "[" + m_indexNode->printDebug() + "]";
	return str;
}

CE::Decompiler::ExprTree::GoarFieldNode::GoarFieldNode(ISdaNode* base, DataType::Structure::Field* field)
	: GoarNode(base), m_field(field)
{}

DataTypePtr CE::Decompiler::ExprTree::GoarFieldNode::getSrcDataType() {
	return m_field->getDataType();
}

ISdaNode* CE::Decompiler::ExprTree::GoarFieldNode::cloneSdaNode(NodeCloneContext* ctx) {
	return new GoarFieldNode(dynamic_cast<ISdaNode*>(m_base->clone()), m_field);
}

std::string CE::Decompiler::ExprTree::GoarFieldNode::printSdaDebug() {
	auto str = m_base->printSdaDebug();
	str += m_base->getDataType()->isPointer() ? "->" : ".";
	str += m_field->getName();
	return str;
}
