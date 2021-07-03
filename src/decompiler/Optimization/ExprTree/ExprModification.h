#pragma once
#include <decompiler/ExprTree/ExprTree.h>

namespace CE::Decompiler
{
	using namespace ExprTree;

	// abstract class for some expr modification/simplification
	class ExprModification
	{
		INode* m_node;
	public:
		ExprModification(INode* node);

		virtual void start() = 0;

		INode* getNode() const;

		bool isChanged() const;

	protected:
		bool m_isChanged = false;

		void changed();

		// replace this node with another
		void replace(INode* newNode, bool destroy = true);
	};
};