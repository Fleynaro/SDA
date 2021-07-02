#pragma once
#include "ExprTreeSdaNode.h"
#include "ExprTreeSdaGenericNode.h"
#include "ExprTreeSdaLeaf.h"

namespace CE::Decompiler::ExprTree
{
	// means some UNNAMED memory location(not value!) presented as linear expr. value: [base] + [term1] + ... + [termN] + constant
	class UnknownLocation : public SdaNode, public INodeAgregator, public PCode::IRelatedToInstruction, public ILocatable
	{
	public:
		struct Term {
			ISdaNode* m_node;
			
			INumberLeaf* getMultiplier() {
				if (auto sdaTermGenNode = dynamic_cast<SdaGenericNode*>(m_node)) {
					if (auto opNode = dynamic_cast<OperationalNode*>(sdaTermGenNode->getNode())) {
						if (opNode->m_operation == Mul) {
							return dynamic_cast<INumberLeaf*>(opNode->m_rightNode);
						}
					}
				}
				return nullptr;
			}
		};

		LinearExpr* m_linearExpr;
		int m_baseNodeIdx;

		UnknownLocation(LinearExpr* linearExpr, int baseNodeIdx);

		~UnknownLocation();

		ISdaNode* getBaseSdaNode();

		LinearExpr* getLinearExpr();

		void setConstTermValue(int64_t constTerm);

		int64_t getConstTermValue();

		std::list<Term> getArrTerms();

		void replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) override;

		std::list<INode*> getNodesList() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		int getSize() override;

		HS getHash() override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		DataTypePtr getSrcDataType() override;

		void setDataType(DataTypePtr dataType) override;

		void getLocation(MemLocation& location) override;

		std::string printSdaDebug() override;
	};
};