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

		UnknownLocation(LinearExpr* linearExpr, int baseNodeIdx)
			: m_linearExpr(linearExpr), m_baseNodeIdx(baseNodeIdx)
		{}

		~UnknownLocation() {
			m_linearExpr->removeBy(this);
		}

		ISdaNode* getBaseSdaNode() {
			int idx = 0;
			for (auto termNode : m_linearExpr->getTerms()) {
				if (idx++ == m_baseNodeIdx)
					return dynamic_cast<ISdaNode*>(termNode);
			}
			return nullptr;
		}

		LinearExpr* getLinearExpr() {
			return m_linearExpr;
		}

		void setConstTermValue(int64_t constTerm) {
			m_linearExpr->setConstTermValue(constTerm);
		}

		int64_t getConstTermValue() {
			return m_linearExpr->getConstTermValue();
		}

		std::list<Term> getArrTerms() {
			std::list<Term> terms;
			int idx = 0;
			for (auto termNode : m_linearExpr->getTerms()) {
				if (idx++ == m_baseNodeIdx)
					continue;
				Term term;
				term.m_node = dynamic_cast<ISdaNode*>(termNode);
				terms.push_back(term);
			}
			return terms;
		}

		void replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) override {
			if (node == m_linearExpr)
				m_linearExpr = dynamic_cast<LinearExpr*>(newNode);
		}

		std::list<INode*> getNodesList() override {
			return m_linearExpr->getNodesList();
		}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			return m_linearExpr->getInstructionsRelatedTo();
		}

		int getSize() override {
			return m_linearExpr->getSize();
		}

		HS getHash() override {
			return m_linearExpr->getHash(); //todo: + term hashes
		}

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override {
			auto clonedLinearExpr = dynamic_cast<LinearExpr*>(m_linearExpr->clone(ctx));
			auto newUnknownLocation = new UnknownLocation(clonedLinearExpr, m_baseNodeIdx);
			clonedLinearExpr->addParentNode(newUnknownLocation);
			return newUnknownLocation;
		}

		DataTypePtr getSrcDataType() override {
			return getBaseSdaNode()->getDataType();
		}

		void setDataType(DataTypePtr dataType) override {
			getBaseSdaNode()->setDataType(dataType);
		}

		void getLocation(MemLocation& location) override {
			auto baseSdaNode = getBaseSdaNode();
			auto valueDataType = CloneUnit(baseSdaNode->getDataType());
			valueDataType->removePointerLevelOutOfFront();

			if (auto locatableNode = dynamic_cast<ILocatable*>(baseSdaNode)) {
				locatableNode->getLocation(location);
			}
			else {
				location.m_type = MemLocation::IMPLICIT;
				location.m_baseAddrHash = baseSdaNode->getHash();
			}
			location.m_offset += getConstTermValue();
			location.m_valueSize = valueDataType->getSize();
			for (auto term : getArrTerms()) {
				auto multiplier = term.getMultiplier();
				auto itemSize = multiplier ? (int)multiplier->getValue() : 1;
				location.addArrayDim(itemSize);
			}
		}

		std::string printSdaDebug() override {
			return m_linearExpr->printDebug();
		}
	};
};