#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	// Removing seq. assignments lines with unknown register
	class GraphUselessLineDeleting : public GraphModification
	{
	public:
		GraphUselessLineDeleting(DecompiledCodeGraph* decGraph)
			: GraphModification(decGraph)
		{}

		void start() override {
			for (auto decBlock : m_decGraph->getDecompiledBlocks()) {
				processBlock(decBlock);
			}
		}
	private:
		void processBlock(DecBlock* block) {
			for (auto seqLine : block->getSeqAssignmentLines()) {
				if (auto symbolLeaf = dynamic_cast<SymbolLeaf*>(seqLine->getDstNode()))
					if (dynamic_cast<Symbol::AbstractVariable*>(symbolLeaf->m_symbol))
						continue;

				if (hasUnknownRegister(seqLine->getAssignmentNode())) {
					delete seqLine;
				}
			}
		}

		bool hasUnknownRegister(INode* node) {
			if (dynamic_cast<FunctionCall*>(node))
				return false;

			bool result = false;
			node->iterateChildNodes( [&](INode* childNode) {
				if (!result)
					result = hasUnknownRegister(childNode);
				});
			if (result)
				return true;

			if (auto symbolLeaf = dynamic_cast<SymbolLeaf*>(node)) {
				if (auto regVar = dynamic_cast<Symbol::RegisterVariable*>(symbolLeaf->m_symbol)) {
					if (regVar->m_register.isPointer())
						return false;
					return isUnknownRegister(regVar->m_register);
				}
			}
			return false;
		}

		bool isUnknownRegister(PCode::Register& reg) {
			bool isFound = false;
			/*for (auto paramInfo : m_decGraph->getFunctionCallInfo().getParamInfos()) {
				if (reg.getId() == paramInfo.m_storage.getRegisterId()) {
					isFound = true;
					break;
				}
			}*/
			return !isFound;
		}
	};
};