#pragma once
#include "decompiler/DecStorage.h"
#include <decompiler/DecMask.h>
#include <decompiler/PCode/DecPCode.h>
#include <utilities/HashSerialization.h>
#include <map>
#include <functional>
#include <stdexcept>

namespace CE::Decompiler::Symbol
{
	class Symbol;
};

namespace CE::Decompiler::ExprTree
{
	struct NodeCloneContext {
		bool m_cloneSymbols = false;
		std::map<Symbol::Symbol*, Symbol::Symbol*> m_clonedSymbols;
	};

	class INode;
	// something that can contain expr. node
	class INodeAgregator
	{
	public:
		virtual void replaceNode(INode* node, INode* newNode) = 0;

		virtual std::list<INode*> getNodesList() = 0;
	};

	class INode
	{
	public:
		virtual ~INode() {}

		virtual void replaceWith(INode* newNode) = 0;

		virtual void removeBy(INodeAgregator* node) = 0;

		virtual void addParentNode(INodeAgregator* node) = 0;

		virtual void removeParentNode(INodeAgregator* node) = 0;

		virtual std::list<INodeAgregator*>& getParentNodes() = 0;

		virtual INodeAgregator* getParentNode() = 0;

		virtual HS getHash() = 0;

		virtual int getSize() = 0;

		virtual bool isFloatingPoint() = 0;

		virtual INode* clone();

		virtual INode* clone(NodeCloneContext* ctx) = 0;

		void iterateChildNodes(std::function<void(INode*)> func);

		virtual void checkOnSingleParents();
	};

	struct StoragePath
	{
		Symbol::Symbol* m_symbol = nullptr;
		std::list<int64_t> m_offsets;
	};
	
	class IStoragePathNode : virtual public INode
	{
	public:
		virtual StoragePath getStoragePath() = 0;
	};

	class Node : public virtual INode
	{
	public:
		std::string m_updateDebugInfo;

		Node()
		{}

		~Node() {
			replaceWith(nullptr);
		}

		// replace this node with another, remove all associations and make this node independent from expression tree
		void replaceWith(INode* newNode) override;

		void removeBy(INodeAgregator* node) override;

		void addParentNode(INodeAgregator* node) override;

		void removeParentNode(INodeAgregator* node) override;

		std::list<INodeAgregator*>& getParentNodes() override;

		// get single parent (exception thrown because of multiple parents)
		INodeAgregator* getParentNode() override;

		// not integer type
		bool isFloatingPoint() override;

	private:
		std::list<INodeAgregator*> m_parentNodes;
	};

	extern void UpdateDebugInfo(INode* node);
};