#pragma once
#include "ExprTreeOperationalNode.h"

namespace CE::Decompiler::ExprTree
{
	class LinearExpr : public Node, public INodeAgregator, public PCode::IRelatedToInstruction
	{
		std::list<INode*> m_terms;
		INumberLeaf* m_constTerm;
	public:
		OperationType m_operation;
		
		LinearExpr(INumberLeaf* constTerm, OperationType operation = Add);

		~LinearExpr();

		void addTerm(ExprTree::INode* term);

		void setConstTermValue(int64_t constTerm) const;

		int64_t getConstTermValue() const;

		std::list<ExprTree::INode*>& getTerms();

		INumberLeaf* getConstTerm() const;

		void replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) override;

		std::list<INode*> getNodesList() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		int getSize() override;

		bool isFloatingPoint() override;

		INode* clone(NodeCloneContext* ctx) override;

		HS getHash() override;

		std::string printDebug() override;
	};
};