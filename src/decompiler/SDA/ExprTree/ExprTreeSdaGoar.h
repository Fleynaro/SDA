#pragma once
#include "ExprTreeSdaNode.h"
#include <datatypes/Structure.h>

namespace CE::Decompiler::ExprTree
{
	// Base class for GoarArrayNode, GoarFieldNode, GoarTopNode
	class GoarNode : public SdaNode, public INodeAgregator, public IStoragePathNode
	{
	public:
		ISdaNode* m_base;

		GoarNode(ISdaNode* base);

		~GoarNode();

		void replaceNode(INode* node, INode* newNode) override;

		std::list<INode*> getNodesList() override;

		int getSize() override;

		HS getHash() override;

		bool isFloatingPoint() override;

		void setDataType(DataTypePtr dataType) override;

	protected:
		StoragePath getNewStoragePath(int64_t offset);
	};

	// Array: players[playersCount - 3]
	class GoarArrayNode : public GoarNode
	{
	public:
		ISdaNode* m_indexNode;
		DataTypePtr m_outDataType;
		int m_itemsMaxCount;

		GoarArrayNode(ISdaNode* base, ISdaNode* indexNode, DataTypePtr dataType, int itemsMaxCount);

		~GoarArrayNode();

		void replaceNode(INode* node, INode* newNode) override;

		std::list<INode*> getNodesList() override;

		DataTypePtr getSrcDataType() override;

		HS getHash() override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		StoragePath getStoragePath() override;
	};

	inline StoragePath GoarArrayNode::getStoragePath() {
		if (const auto numberLeaf = dynamic_cast<INumberLeaf*>(m_indexNode)) {
			return getNewStoragePath(numberLeaf->getValue() * m_outDataType->getSize());
		}
		return StoragePath();
	}

	// Field of a class: player.pos.x
	class GoarFieldNode : public GoarNode
	{
	public:
		CE::Symbol::StructFieldSymbol* m_field;

		GoarFieldNode(ISdaNode* base, CE::Symbol::StructFieldSymbol* field);

		DataTypePtr getSrcDataType() override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		StoragePath getStoragePath() override;
	};

	inline StoragePath GoarFieldNode::getStoragePath() {
		return getNewStoragePath(m_field->getOffset());
	}

	// Top node for any goar structure that need to define operator &: &player.pos.x
	class GoarTopNode : public GoarNode, public IMappedToMemory
	{
		bool m_isAddrGetting;
		int64_t m_bitOffset;
	public:
		GoarTopNode(ISdaNode* base, int64_t bitOffset, bool isAddrGetting);

		bool isAddrGetting() override;

		void setAddrGetting(bool toggle) override;

		bool getLocation(MemLocation& location) override;

		DataTypePtr getSrcDataType() override;

		HS getHash() override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		StoragePath getStoragePath() override {
			return StoragePath();
		}

	private:
		// players[2][10] -> dims: 10, 2
		void gatherArrDims(ISdaNode* node, MemLocation& location);

		// for players[0].pos.x the base is "players" array
		ISdaNode* getBaseNode(ISdaNode* node);
	};
};