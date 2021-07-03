#include "ExprTreeSdaNode.h"

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::SdaTopNode::SdaTopNode(ISdaNode* node)
	: TopNode(node)
{}

ISdaNode* CE::Decompiler::ExprTree::SdaTopNode::getSdaNode() {
	return dynamic_cast<ISdaNode*>(getNode());
}

DataTypePtr CE::Decompiler::ExprTree::DataTypeCast::getCastDataType() const
{
	return m_castDataType;
}

bool CE::Decompiler::ExprTree::DataTypeCast::hasExplicitCast() const
{
	return m_explicitCast;
}

void CE::Decompiler::ExprTree::DataTypeCast::setCastDataType(DataTypePtr dataType, bool isExplicit) {
	m_castDataType = dataType;
	m_explicitCast = isExplicit;
}

void CE::Decompiler::ExprTree::DataTypeCast::clearCast() {
	setCastDataType(nullptr, false);
}

DataTypePtr CE::Decompiler::ExprTree::ISdaNode::getDataType() {
	return hasCast() ? getCast()->getCastDataType() : getSrcDataType();
}

bool CE::Decompiler::ExprTree::ISdaNode::hasCast() {
	return getCast()->getCastDataType() != nullptr;
}

std::string CE::Decompiler::ExprTree::ISdaNode::printSdaDebug() {
	return "";
}

DataTypeCast* CE::Decompiler::ExprTree::SdaNode::getCast() {
	return &m_dataTypeCast;
}

INode* CE::Decompiler::ExprTree::SdaNode::clone(NodeCloneContext* ctx) {
	auto clonedSdaNode = cloneSdaNode(ctx);
	clonedSdaNode->getCast()->setCastDataType(getCast()->getCastDataType(), getCast()->hasExplicitCast());
	return clonedSdaNode;
}

std::string CE::Decompiler::ExprTree::SdaNode::printDebug() {
	auto result = printSdaDebug();
	if (auto addressGetting = dynamic_cast<IMappedToMemory*>(this))
		if (addressGetting->isAddrGetting())
			result = "&" + result;
	if (hasCast() && getCast()->hasExplicitCast()) {
		result = "(" + getCast()->getCastDataType()->getDisplayName() + ")" + result + "";
	}
	if (g_MARK_SDA_NODES)
		result = "@" + result;
	return m_updateDebugInfo = result;
}
