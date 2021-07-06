#pragma once
#include <functional>
#include "Control.h"

namespace GUI
{
	// used for mapping trees to the tree model format that can be drawn
	template<typename T>
	class ITreeModel
	{
	public:
		class Iterator;
		using IteratorCallback = std::function<void(Iterator*)>;

		// iterator interface
		class Iterator
		{
		public:
			virtual void getNextItem(std::string* text, T* data, const IteratorCallback& callback) = 0;

			virtual bool hasNextItem() = 0;
		};

		virtual void newIterator(const IteratorCallback& callback) = 0;
	};

	// standard list model that uses internally Node and std::list for allocating new nodes
	template<typename T>
	class StdTreeModel : public ITreeModel<T>
	{
		struct Node
		{
			std::string m_text;
			T m_data;
			std::list<Node*> m_childNodes;
		};
		Node m_rootNode;
		std::list<Node> m_nodes;
	public:
		class StdIterator : public ITreeModel<T>::Iterator
		{
			typename std::list<Node*>::iterator m_it;
			Node* m_node;
		public:
			StdIterator(Node* node)
				: m_node(node), m_it(node->m_childNodes.begin())
			{}

			void getNextItem(std::string* text, T* data, const IteratorCallback& callback) override
			{
				*text = (*m_it)->m_text;
				*data = (*m_it)->m_data;
				StdIterator iterator(*m_it);
				callback(&iterator);
				++m_it;
			}

			bool hasNextItem() override
			{
				return m_it != m_node->m_childNodes.end();
			}
		};
		
		StdTreeModel()
		{
			Node rootNode;
			rootNode.m_text = "root node";
			m_rootNode = rootNode;
		}

		Node* getRootNode()
		{
			return &m_rootNode;
		}

		Node* createNode(const std::string& text, T data)
		{
			Node newNode;
			newNode.m_text = text;
			newNode.m_data = data;
			m_nodes.push_back(newNode);
			return &*m_nodes.rbegin();
		}

		void clear()
		{
			m_nodes.clear();
		}

		void newIterator(const IteratorCallback& callback) override
		{
			StdIterator iterator(&m_rootNode);
			callback(&iterator);
		}
	};

	// used to draw a tree based on a tree model
	template<typename T>
	class AbstractTreeView
		: public Control
	{
		ITreeModel<T>* m_treeModel;
	public:
		AbstractTreeView(ITreeModel<T>* treeModel = nullptr)
			: m_treeModel(treeModel)
		{}

	private:
		void renderControl() override
		{
			m_treeModel->newIterator([&](typename ITreeModel<T>::Iterator* iter)
				{
					renderTree(iter);
				});
		}

	protected:
		using Iterator = typename ITreeModel<T>::Iterator;

		// render a tree (list of nodes)
		virtual void renderTree(Iterator* iter)
		{
			while (iter->hasNextItem())
			{
				std::string text;
				T data;
				iter->getNextItem(&text, &data, [&](Iterator* iter)
					{
						renderNode(text, data, iter);
					});
			}
		}

		// render a node that can contains child nodes
		virtual void renderNode(const std::string& text, const T& data, Iterator* iter) = 0;
	};

	template<typename T>
	class StdTreeView
		: public AbstractTreeView<T>
	{
		EventHandler<T> m_clickItemEventHandler;
	public:
		StdTreeView(ITreeModel<T>* treeModel = nullptr)
			: AbstractTreeView<T>(treeModel)
		{}

		void handler(const std::function<void(T)>& clickItemEventHandler)
		{
			m_clickItemEventHandler = clickItemEventHandler;
		}

	protected:
		void renderNode(const std::string& text, const T& data, Iterator* iter) override
		{
			if (iter->hasNextItem())
			{
				if (ImGui::TreeNode(text.c_str()))
				{
					renderTree(iter);
					ImGui::TreePop();
				}
			}
			else {
				if (ImGui::Selectable(text.c_str())) {
					m_clickItemEventHandler(data);
				}
			}
		}
	};
};