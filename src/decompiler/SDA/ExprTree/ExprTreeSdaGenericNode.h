#pragma once
#include "ExprTreeSdaNode.h"

namespace CE::Decompiler::ExprTree
{
	class SdaGenericNode : public SdaNode, public INodeAgregator
	{
		DataTypePtr m_calcDataType;
		INode* m_node;
	public:
		SdaGenericNode(INode* node, DataTypePtr calcDataType);

		~SdaGenericNode();

		void replaceNode(INode* node, INode* newNode) override;

		std::list<ExprTree::INode*> getNodesList() override;

		INode* getNode() const;

		DataTypePtr getSrcDataType() override;

		void setDataType(DataTypePtr dataType) override;

		int getSize() override;

		bool isFloatingPoint() override;

		HS getHash() override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		std::string printSdaDebug() override;
	};
};