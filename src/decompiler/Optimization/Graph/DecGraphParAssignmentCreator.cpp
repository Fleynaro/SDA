#include "DecGraphParAssignmentCreator.h"

using namespace CE::Decompiler;

// optimize: localVar = ((5 << 32) | 10) & 0xFFFFFFFF -> localVar = 10

Optimization::GraphParAssignmentCreator::GraphParAssignmentCreator(DecompiledCodeGraph* decGraph, PrimaryDecompiler* decompiler)
	: GraphModification(decGraph), m_decompiler(decompiler)
{}

void Optimization::GraphParAssignmentCreator::start() {
	// iterate over all expr. in dec. graph
	for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
		for (auto topNode : decBlock->getAllTopNodes()) {
			findAllLocalVarsAndGatherParentOpNodes(topNode);
		}
	}

	// create assignments
	createParAssignmentsForLocalVars();
	// optimize them
	optimizeAllParAssignments();
}

void Optimization::GraphParAssignmentCreator::findAllLocalVarsAndGatherParentOpNodes(DecBlock::BlockTopNode* topNode) {
	std::list<SymbolLeaf*> symbolLeafs;
	GatherSymbolLeafsFromNode(topNode->getNode(), symbolLeafs);
	for (auto symbolLeaf : symbolLeafs) {
		if (auto localVar = dynamic_cast<Symbol::LocalVariable*>(symbolLeaf->m_symbol))
		{
			if (m_localVars.find(localVar) == m_localVars.end())
				m_localVars[localVar] = LocalVarInfo();

			// ignore localVars in dst node of assignments: localVar = 5
			const auto parentAssignmentNode = dynamic_cast<AssignmentNode*>(symbolLeaf->getParentNode());
			if (parentAssignmentNode && parentAssignmentNode->getDstNode() == symbolLeaf)
				continue;

			// see parent
			bool isParentOpNode = false;
			if (auto opNode = dynamic_cast<OperationalNode*>(symbolLeaf->getParentNode())) {
				if (opNode->m_operation == And) {
					/*if (const auto parentOpNode = dynamic_cast<OperationalNode*>(opNode->getParentNode())) { // for sample #50
						if (parentOpNode->m_operation == Shr) {
							m_localVars[localVar].m_opNodes.push_back(parentOpNode);
							isParentOpNode = true;
						}
					}*/
					if (!isParentOpNode) {
						const bool isSymbolInLeftNode = (opNode->m_leftNode == symbolLeaf);
						m_localVars[localVar].m_opNodes.push_back(isSymbolInLeftNode ? opNode->m_rightNode : opNode->m_leftNode);
						isParentOpNode = true;
					}
				}
			}
			m_localVars[localVar].areAllParentOpNode &= isParentOpNode;
		}
	}
}

void Optimization::GraphParAssignmentCreator::createParAssignmentsForLocalVars() {
	for (const auto& [localVar, info] : m_localVars) {
		auto& localVarInfo = m_decompiler->m_localVars[localVar];
		localVarInfo.m_used = true;

		// try to change the size of the local var
		if (info.areAllParentOpNode) {
			ExprBitMask mask;
			for (const auto opNode : info.m_opNodes) {
				mask = mask | CalculateMask(opNode);
			}

			// change the size of local var
			const auto newLocalVarSize = mask.getSize();
			if (newLocalVarSize != 0 && newLocalVarSize != localVar->getSize()) {
				localVar->setSize(newLocalVarSize);
				localVarInfo.m_register.m_valueRangeMask = BitMask64(newLocalVarSize);
				// optimize all parent operational AND nodes
				for (const auto opNode : info.m_opNodes) {
					auto topNode = TopNode(opNode);
					ExprOptimization exprOptimization(&topNode);
					exprOptimization.start();
				}
			}
		}

		// iterate over all ctxs and create assignments: localVar1 = 0x5
		for (auto execCtx : localVarInfo.m_parentExecCtxs) {
			const auto expr = execCtx->m_registerExecCtx.requestRegister(localVarInfo.m_register);

			// to avoide: localVar1 = localVar1
			if (const auto symbolLeaf = dynamic_cast<SymbolLeaf*>(expr))
				if (symbolLeaf->m_symbol == localVar)
					continue;

			// associate localVar with PCode instructions
			if (auto nodeRelToInstr = dynamic_cast<IRelatedToInstruction*>(expr)) {
				for (auto instr : nodeRelToInstr->getInstructionsRelatedTo())
					localVar->m_instructionsRelatedTo.push_back(instr);
			}

			// create assignment: localVar = {expr}
			auto& blockInfo = m_decompiler->m_decompiledBlocks[execCtx->m_pcodeBlock];
			blockInfo.m_decBlock->addSymbolParallelAssignmentLine(new SymbolLeaf(localVar), expr);
		}

		m_decGraph->addSymbol(localVar);
	}
}

void Optimization::GraphParAssignmentCreator::optimizeAllParAssignments() const
{
	for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
		for (auto parAssignmentLine : decBlock->getSymbolParallelAssignmentLines()) {
			auto assignmentNode = parAssignmentLine->getAssignmentNode();

			// clone and optimize expr
			const auto clonedExpr = assignmentNode->getSrcNode()->clone();
			auto topNode = TopNode(clonedExpr);
			ExprOptimization exprOptimization(&topNode);
			exprOptimization.start();

			assignmentNode->setSrcNode(topNode.getNode());
		}
	}
}
