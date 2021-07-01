#pragma once
#include "../DecMask.h"
#include "../PCode/DecPCode.h"
#include <utilities/HashSerialization.h>

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

		virtual INode* clone() {
			NodeCloneContext ctx;
			return clone(&ctx);
		}

		virtual INode* clone(NodeCloneContext* ctx) = 0;

		void iterateChildNodes(std::function<void(INode*)> func) {
			if (auto agregator = dynamic_cast<INodeAgregator*>(this)) {
				auto list = agregator->getNodesList();
				for (auto node : list) {
					if (node) {
						func(node);
					}
				}
			}
		}

		virtual std::string printDebug() = 0;

		void static UpdateDebugInfo(INode* node) {
			if (!node) return;
			node->printDebug();
			//node->checkOnSingleParents();
		}

		virtual void checkOnSingleParents() {
			auto parentNode = getParentNode();
			if (auto nodeAgregator = dynamic_cast<INodeAgregator*>(this)) {
				for (auto childNode : nodeAgregator->getNodesList())
					if(childNode)
						childNode->checkOnSingleParents();
			}
		}
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
		void replaceWith(INode* newNode) override {
			for (auto it = m_parentNodes.begin(); it != m_parentNodes.end(); it ++) {
				auto parentNode = *it;
				if (newNode == dynamic_cast<INode*>(parentNode))
					continue;
				parentNode->replaceNode(this, newNode);
				if (newNode != nullptr) {
					newNode->addParentNode(parentNode);
				}
				m_parentNodes.erase(it);
			}
		}

		void removeBy(INodeAgregator* node) override {
			if (node != nullptr) {
				node->replaceNode(this, nullptr);
				removeParentNode(node);
			}
			if (m_parentNodes.size() == 0)
				delete this;
		}

		void addParentNode(INodeAgregator* node) override {
			if (this == dynamic_cast<INode*>(node))
				return;
			m_parentNodes.push_back(node);
		}

		void removeParentNode(INodeAgregator* node) override {
			m_parentNodes.remove(node);
		}

		std::list<INodeAgregator*>& getParentNodes() override {
			return m_parentNodes;
		}

		// get single parent (exception thrown because of multiple parents)
		INodeAgregator* getParentNode() override {
			if (m_parentNodes.size() != 1) {
				throw std::logic_error("it is ambigious because of multiple parents");
			}
			return *m_parentNodes.begin();
		}

		// not integer type
		bool isFloatingPoint() override {
			return false;
		}

		std::string printDebug() override {
			return "";
		}

	private:
		std::list<INodeAgregator*> m_parentNodes;
	};
};