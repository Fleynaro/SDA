#pragma once
#include "ExprTreeSdaNode.h"
#include <datatypes/FunctionSignature.h>
#include <decompiler/ExprTree/ExprTreeFunctionCall.h>

namespace CE::Decompiler::ExprTree
{
	class SdaFunctionNode : public SdaNode, public INodeAgregator
	{
		FunctionCall* m_funcCall;

	public:
		SdaFunctionNode(FunctionCall* funcCallCtx);

		~SdaFunctionNode();

		void replaceNode(INode* node, INode* newNode) override;

		std::list<ExprTree::INode*> getNodesList() override;

		// means the address of the function that can be any expr. value, not only an offset or a symbol
		INode* getDestination();

		std::vector<ExprTree::INode*>& getParamNodes();

		DataTypePtr getSrcDataType() override;

		void setDataType(DataTypePtr dataType) override;

		int getSize() override;

		bool isFloatingPoint() override;

		HS getHash() override;

		int64_t getCallInstrOffset();

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		// example: (world->vtable->func_get_player)(player_id) where {world->vtable->func_get_player} has a signature type calculated through the step of goar building
		DataType::IFunctionSignature* getSignature();

		std::string printSdaDebug() override;
	};
};