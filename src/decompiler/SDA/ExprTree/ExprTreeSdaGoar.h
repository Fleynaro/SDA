#pragma once
#include "ExprTreeSdaNode.h"

namespace CE::Decompiler::ExprTree
{
	// Base class for GoarArrayNode, GoarFieldNode, GoarTopNode
	class GoarNode : public SdaNode, public INodeAgregator
	{
	public:
		ISdaNode* m_base;

		GoarNode(ISdaNode* base)
			: m_base(base)
		{
			m_base->addParentNode(this);
		}

		~GoarNode() {
			m_base->removeBy(this);
		}

		void replaceNode(INode* node, INode* newNode) override {
			auto newSdaNode = dynamic_cast<ISdaNode*>(newNode);
			if (node == m_base) {
				m_base = newSdaNode;
			}
		}

		std::list<ExprTree::INode*> getNodesList() override {
			return { m_base };
		}

		int getSize() override {
			return getSrcDataType()->getSize();
		}

		HS getHash() override {
			return m_base->getHash();
		}

		bool isFloatingPoint() override {
			return false;
		}

		void setDataType(DataTypePtr dataType) override {
		}
	};

	// Array: players[playersCount - 3]
	class GoarArrayNode : public GoarNode
	{
	public:
		ISdaNode* m_indexNode;
		DataTypePtr m_outDataType;
		int m_itemsMaxCount;

		GoarArrayNode(ISdaNode* base, ISdaNode* indexNode, DataTypePtr dataType, int itemsMaxCount)
			: GoarNode(base), m_indexNode(indexNode), m_outDataType(dataType), m_itemsMaxCount(itemsMaxCount)
		{
			m_indexNode->addParentNode(this);
		}

		~GoarArrayNode() {
			m_indexNode->removeBy(this);
		}

		void replaceNode(INode* node, INode* newNode) override {
			GoarNode::replaceNode(node, newNode);
			auto newSdaNode = dynamic_cast<ISdaNode*>(newNode);
			if (node == m_indexNode) {
				m_indexNode = newSdaNode;
			}
		}

		std::list<ExprTree::INode*> getNodesList() override {
			return { m_base, m_indexNode };
		}

		DataTypePtr getSrcDataType() override {
			return m_outDataType;
		}

		HS getHash() override {
			return GoarNode::getHash() << m_indexNode->getHash();
		}

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override {
			return new GoarArrayNode(dynamic_cast<ISdaNode*>(m_base->clone()), dynamic_cast<ISdaNode*>(m_indexNode->clone(ctx)), CloneUnit(m_outDataType), m_itemsMaxCount);
		}

		std::string printSdaDebug() override {
			auto str = m_base->printSdaDebug();
			str = str + "[" + m_indexNode->printDebug() + "]";
			return str;
		}
	};

	// Field of a class: player.pos.x
	class GoarFieldNode : public GoarNode
	{
	public:
		DataType::Structure::Field* m_field;

		GoarFieldNode(ISdaNode* base, DataType::Structure::Field* field)
			: GoarNode(base), m_field(field)
		{}

		DataTypePtr getSrcDataType() override {
			return m_field->getDataType();
		}

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override {
			return new GoarFieldNode(dynamic_cast<ISdaNode*>(m_base->clone()), m_field);
		}

		std::string printSdaDebug() override {
			auto str = m_base->printSdaDebug();
			str += m_base->getDataType()->isPointer() ? "->" : ".";
			str += m_field->getName();
			return str;
		}
	};

	// Top node for any goar structure that need to define operator &: &player.pos.x
	class GoarTopNode : public GoarNode, public IMappedToMemory
	{
		bool m_isAddrGetting;
		int64_t m_bitOffset;
	public:
		GoarTopNode(ISdaNode* base, int64_t bitOffset, bool isAddrGetting)
			: GoarNode(base), m_bitOffset(bitOffset), m_isAddrGetting(isAddrGetting)
		{}

		bool isAddrGetting() override {
			return m_isAddrGetting;
		}

		void setAddrGetting(bool toggle) override {
			m_isAddrGetting = toggle;
		}

		void getLocation(MemLocation& location) override {
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

		DataTypePtr getSrcDataType() override {
			if (m_isAddrGetting) {
				return MakePointer(m_base->getDataType());
			}
			return m_base->getDataType();
		}

		HS getHash() override {
			return GoarNode::getHash() << m_isAddrGetting;
		}

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override {
			return new GoarTopNode(dynamic_cast<ISdaNode*>(m_base->clone()), m_bitOffset, m_isAddrGetting);
		}
		
		std::string printSdaDebug() override {
			return m_base->printSdaDebug();
		}

	private:
		// players[2][10] -> dims: 10, 2
		void gatherArrDims(ISdaNode* node, MemLocation& location) {
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

		// for players[0].pos.x the base is "players" array
		ISdaNode* getBaseNode(ISdaNode* node) {
			if (auto goarNode = dynamic_cast<GoarNode*>(node)) {
				return getBaseNode(goarNode->m_base);
			}
			return node;
		}
	};
};