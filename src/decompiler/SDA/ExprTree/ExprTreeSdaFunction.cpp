#include "ExprTreeSdaFunction.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::SdaFunctionNode::SdaFunctionNode(FunctionCall* funcCallCtx)
	: m_funcCall(funcCallCtx)
{}

CE::Decompiler::ExprTree::SdaFunctionNode::~SdaFunctionNode() {
	m_funcCall->removeBy(this);
}

void CE::Decompiler::ExprTree::SdaFunctionNode::replaceNode(INode* node, INode* newNode) {
	m_funcCall->replaceNode(node, newNode);
}

std::list<ExprTree::INode*> CE::Decompiler::ExprTree::SdaFunctionNode::getNodesList() {
	return m_funcCall->getNodesList();
}

// means the address of the function that can be any expr. value, not only an offset or a symbol

INode* CE::Decompiler::ExprTree::SdaFunctionNode::getDestination() {
	return m_funcCall->getDestination();
}

std::vector<ExprTree::INode*>& CE::Decompiler::ExprTree::SdaFunctionNode::getParamNodes() {
	return m_funcCall->getParamNodes();
}

DataTypePtr CE::Decompiler::ExprTree::SdaFunctionNode::getSrcDataType() {
	auto sig = getSignature();
	if (!sig)
		return DataType::GetUnit(sig->getTypeManager()->findTypeById(DataType::SystemType::Byte));
	return getSignature()->getReturnType();
}

void CE::Decompiler::ExprTree::SdaFunctionNode::setDataType(DataTypePtr dataType) {
	auto sig = getSignature();
	if (!sig || !sig->isAuto())
		return;
	sig->setReturnType(dataType);
}

int CE::Decompiler::ExprTree::SdaFunctionNode::getSize() {
	return m_funcCall->getSize();
}

bool CE::Decompiler::ExprTree::SdaFunctionNode::isFloatingPoint() {
	return m_funcCall->isFloatingPoint();
}

HS CE::Decompiler::ExprTree::SdaFunctionNode::getHash() {
	return m_funcCall->getHash();
}

int64_t CE::Decompiler::ExprTree::SdaFunctionNode::getCallInstrOffset() {
	return m_funcCall->m_instr->getOffset();
}

ISdaNode* CE::Decompiler::ExprTree::SdaFunctionNode::cloneSdaNode(NodeCloneContext* ctx) {
	auto clonedFuncCall = dynamic_cast<FunctionCall*>(m_funcCall->clone(ctx));
	auto sdaFunctionNode = new SdaFunctionNode(clonedFuncCall);
	clonedFuncCall->addParentNode(sdaFunctionNode);
	return sdaFunctionNode;
}


// example: (world->vtable->func_get_player)(player_id) where {world->vtable->func_get_player} has a signature type calculated through the step of goar building

DataType::IFunctionSignature* CE::Decompiler::ExprTree::SdaFunctionNode::getSignature() {
	if (auto dstCastNode = dynamic_cast<ISdaNode*>(getDestination()))
		if (auto signature = dynamic_cast<DataType::IFunctionSignature*>(dstCastNode->getDataType()->getType()))
			return signature;
	return nullptr;
}

std::string CE::Decompiler::ExprTree::SdaFunctionNode::printSdaDebug() {
	return m_funcCall->printDebug();
}
