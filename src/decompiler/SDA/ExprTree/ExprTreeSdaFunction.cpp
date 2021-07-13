#include "ExprTreeSdaFunction.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace Decompiler;
using namespace ExprTree;

SdaFunctionNode::SdaFunctionNode(FunctionCall* funcCallCtx)
	: m_funcCall(funcCallCtx)
{}

SdaFunctionNode::~SdaFunctionNode() {
	m_funcCall->removeBy(this);
}

void SdaFunctionNode::replaceNode(INode* node, INode* newNode) {
	m_funcCall->replaceNode(node, newNode);
}

std::list<INode*> SdaFunctionNode::getNodesList() {
	return m_funcCall->getNodesList();
}

// means the address of the function that can be any expr. value, not only an offset or a symbol

INode* SdaFunctionNode::getDestination() const
{
	return m_funcCall->getDestination();
}

std::vector<INode*>& SdaFunctionNode::getParamNodes() const
{
	return m_funcCall->getParamNodes();
}

DataTypePtr SdaFunctionNode::getSrcDataType() {
	const auto sig = getSignature();
	if (!sig)
		// todo: return DataType::GetUnit(sig->getTypeManager()->findTypeById(DataType::SystemType::Byte));
		return GetUnit(new DataType::Byte);
	return getSignature()->getReturnType();
}

void SdaFunctionNode::setDataType(DataTypePtr dataType) {
	auto sig = getSignature();
	if (!sig || !sig->isAuto())
		return;
	sig->setReturnType(dataType);
}

int SdaFunctionNode::getSize() {
	return m_funcCall->getSize();
}

bool SdaFunctionNode::isFloatingPoint() {
	return m_funcCall->isFloatingPoint();
}

HS SdaFunctionNode::getHash() {
	return m_funcCall->getHash();
}

int64_t SdaFunctionNode::getCallInstrOffset() const
{
	return m_funcCall->m_instr->getOffset();
}

ISdaNode* SdaFunctionNode::cloneSdaNode(NodeCloneContext* ctx) {
	auto clonedFuncCall = dynamic_cast<FunctionCall*>(m_funcCall->clone(ctx));
	const auto sdaFunctionNode = new SdaFunctionNode(clonedFuncCall);
	clonedFuncCall->addParentNode(sdaFunctionNode);
	return sdaFunctionNode;
}


// example: (world->vtable->func_get_player)(player_id) where {world->vtable->func_get_player} has a signature type calculated through the step of goar building

DataType::IFunctionSignature* SdaFunctionNode::getSignature() const
{
	if (auto dstCastNode = dynamic_cast<ISdaNode*>(getDestination()))
		if (const auto signature = dynamic_cast<DataType::IFunctionSignature*>(dstCastNode->getDataType()->getType()))
			return signature;
	return nullptr;
}

std::string SdaFunctionNode::printSdaDebug() {
	return m_funcCall->printDebug();
}
