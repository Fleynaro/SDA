#include "ExprTreeSdaNode.h"

using namespace CE;
using namespace Decompiler;
using namespace ExprTree;

SdaTopNode::SdaTopNode(ISdaNode* node)
	: TopNode(node)
{}

ISdaNode* SdaTopNode::getSdaNode() {
	return dynamic_cast<ISdaNode*>(getNode());
}

DataTypePtr DataTypeCast::getCastDataType() const
{
	return m_castDataType;
}

bool DataTypeCast::hasExplicitCast() const
{
	return m_explicitCast;
}

void DataTypeCast::setCastDataType(DataTypePtr dataType, bool isExplicit) {
	m_castDataType = dataType;
	m_explicitCast = isExplicit;
}

void DataTypeCast::clearCast() {
	setCastDataType(nullptr, false);
}

DataTypePtr ISdaNode::getDataType() {
	return hasCast() ? getCast()->getCastDataType() : getSrcDataType();
}

bool ISdaNode::hasCast() {
	return getCast()->getCastDataType() != nullptr;
}

DataTypeCast* SdaNode::getCast() {
	return &m_dataTypeCast;
}

INode* SdaNode::clone(NodeCloneContext* ctx) {
	auto clonedSdaNode = cloneSdaNode(ctx);
	clonedSdaNode->getCast()->setCastDataType(getCast()->getCastDataType(), getCast()->hasExplicitCast());
	return clonedSdaNode;
}