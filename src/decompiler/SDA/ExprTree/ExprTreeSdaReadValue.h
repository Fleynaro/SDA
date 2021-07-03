#pragma once
#include "ExprTreeSdaNode.h"

namespace CE::Decompiler::ExprTree
{
	// *(float*)&globalVar (where *(float*) is reader from the specified mem. location and &globalVar is named address)
	class SdaReadValueNode : public SdaNode, public INodeAgregator, public PCode::IRelatedToInstruction, public IMappedToMemory
	{
	public:
		ReadValueNode* m_readValueNode;
		DataTypePtr m_outDataType;

		SdaReadValueNode(ReadValueNode* readValueNode, DataTypePtr outDataType);

		~SdaReadValueNode();

		ISdaNode* getAddress() const;

		void replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) override;

		std::list<INode*> getNodesList() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		int getSize() override;

		HS getHash() override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		DataTypePtr getSrcDataType() override;

		void setDataType(DataTypePtr dataType) override;

		bool isAddrGetting() override;

		void setAddrGetting(bool toggle) override;

		void getLocation(MemLocation& location) override;

		std::string printSdaDebug() override;
	};
};