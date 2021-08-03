#pragma once
#include "ExprTreeOperationalNode.h"

namespace CE::Decompiler::ExprTree
{
	// todo: inherit from OperationalNode to make its processing single through one interface
	class LinearExpr : public Node, public IOperation, public INodeAgregator, public PCode::IRelatedToInstruction
	{
		friend class ExprTreeViewGenerator;
		std::list<INode*> m_terms;
		INumberLeaf* m_constTerm;
	public:
		OperationType m_operation;
		std::list<PCode::Instruction*> m_instructions;
		
		LinearExpr(INumberLeaf* constTerm, OperationType operation = Add);

		~LinearExpr();

		OperationType getOperation() override;

		void addTerm(INode* term);

		void setConstTermValue(int64_t constTerm) const;

		int64_t getConstTermValue() const;

		std::list<INode*>& getTerms();

		INumberLeaf* getConstTerm() const;

		void replaceNode(INode* node, INode* newNode) override;

		std::list<INode*> getNodesList() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		int getSize() override;

		bool isFloatingPoint() override;

		INode* clone(NodeCloneContext* ctx) override;

		HS getHash() override;
	};
};