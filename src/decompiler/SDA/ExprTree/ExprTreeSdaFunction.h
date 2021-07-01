#pragma once
#include "ExprTreeSdaNode.h"
#include <datatypes/FunctionSignature.h>
#include "../../ExprTree/ExprTreeFunctionCall.h"

namespace CE::Decompiler::ExprTree
{
	class SdaFunctionNode : public SdaNode, public INodeAgregator
	{
		FunctionCall* m_funcCall;

	public:
		SdaFunctionNode(FunctionCall* funcCallCtx)
			: m_funcCall(funcCallCtx)
		{}

		~SdaFunctionNode() {
			m_funcCall->removeBy(this);
		}

		void replaceNode(INode* node, INode* newNode) override {
			m_funcCall->replaceNode(node, newNode);
		}

		std::list<ExprTree::INode*> getNodesList() override {
			return m_funcCall->getNodesList();
		}

		// means the address of the function that can be any expr. value, not only an offset or a symbol
		INode* getDestination() {
			return m_funcCall->getDestination();
		}

		std::vector<ExprTree::INode*>& getParamNodes() {
			return m_funcCall->getParamNodes();
		}

		DataTypePtr getSrcDataType() override {
			auto sig = getSignature();
			if (!sig)
				return DataType::GetUnit(new DataType::Byte);
			return getSignature()->getReturnType();
		}

		void setDataType(DataTypePtr dataType) override {
			auto sig = getSignature();
			if (!sig || !sig->isAuto())
				return;
			sig->setReturnType(dataType);
		}

		int getSize() override {
			return m_funcCall->getSize();
		}

		bool isFloatingPoint() override {
			return m_funcCall->isFloatingPoint();
		}

		HS getHash() override {
			return m_funcCall->getHash();
		}

		int64_t getCallInstrOffset() {
			return m_funcCall->m_instr->getOffset();
		}

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override {
			auto clonedFuncCall = dynamic_cast<FunctionCall*>(m_funcCall->clone(ctx));
			auto sdaFunctionNode = new SdaFunctionNode(clonedFuncCall);
			clonedFuncCall->addParentNode(sdaFunctionNode);
			return sdaFunctionNode;
		}

		// example: (world->vtable->func_get_player)(player_id) where {world->vtable->func_get_player} has a signature type calculated through the step of goar building
		DataType::IFunctionSignature* getSignature() {
			if (auto dstCastNode = dynamic_cast<ISdaNode*>(getDestination()))
				if (auto signature = dynamic_cast<DataType::IFunctionSignature*>(dstCastNode->getDataType()->getType()))
					return signature;
			return nullptr;
		}

		std::string printSdaDebug() override {
			return m_funcCall->printDebug();
		}
	};
};