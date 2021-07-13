#pragma once
#include "ExprTreeOperationalNode.h"

namespace CE::Decompiler::ExprTree
{
	class FunctionCall : public Node, public INodeAgregator, public PCode::IRelatedToInstruction
	{
	public:
		INode* m_destination;
		std::vector<INode*> m_paramNodes;
		PCode::Instruction* m_instr;
		Symbol::FunctionResultVar* m_functionResultVar = nullptr;
		
		FunctionCall(INode* destination, PCode::Instruction* instr);

		~FunctionCall();

		void replaceNode(INode* node, INode* newNode) override;

		std::list<INode*> getNodesList() override;

		INode* getDestination() const;

		std::vector<INode*>& getParamNodes();

		void addParamNode(INode* node);

		int getSize() override;

		bool isFloatingPoint() override;

		HS getHash() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		INode* clone(NodeCloneContext* ctx) override;

		std::string printDebug() override;
	};
};