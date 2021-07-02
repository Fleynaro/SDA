#include "DecGraphParAssignmentCreator.h"

using namespace CE::Decompiler;

// optimize: localVar = ((5 << 32) | 10) & 0xFFFFFFFF -> localVar = 10

CE::Decompiler::Optimization::GraphParAssignmentCreator::GraphParAssignmentCreator(DecompiledCodeGraph* decGraph, PrimaryDecompiler* decompiler)
	: GraphModification(decGraph), m_decompiler(decompiler)
{}

void CE::Decompiler::Optimization::GraphParAssignmentCreator::start() {
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

void CE::Decompiler::Optimization::GraphParAssignmentCreator::findAllLocalVarsAndGatherParentOpNodes(DecBlock::BlockTopNode* topNode) {
	std::list<ExprTree::SymbolLeaf*> symbolLeafs;
	GatherSymbolLeafsFromNode(topNode->getNode(), symbolLeafs);
	for (auto symbolLeaf : symbolLeafs) {
		if (auto localVar = dynamic_cast<Symbol::LocalVariable*>(symbolLeaf->m_symbol))
		{
			if (m_localVars.find(localVar) == m_localVars.end())
				m_localVars[localVar] = LocalVarInfo();

			// ignore localVars in dst node of assignments: localVar = 5
			auto parentAssignmentNode = dynamic_cast<ExprTree::AssignmentNode*>(symbolLeaf->getParentNode());
			if (parentAssignmentNode && parentAssignmentNode->getDstNode() == symbolLeaf)
				continue;

			// see parent
			bool isParentOpNode = false;
			if (auto opNode = dynamic_cast<ExprTree::OperationalNode*>(symbolLeaf->getParentNode())) {
				if (opNode->m_operation == ExprTree::And) {
					bool isSymbolInLeftNode = (opNode->m_leftNode == symbolLeaf);
					m_localVars[localVar].m_opNodes.push_back(std::pair(opNode, isSymbolInLeftNode));
					isParentOpNode = true;
				}
			}
			m_localVars[localVar].areAllParentOpNode &= isParentOpNode;
		}
	}
}

void CE::Decompiler::Optimization::GraphParAssignmentCreator::createParAssignmentsForLocalVars() {
	for (auto& pair : m_localVars) {
		auto localVar = pair.first;
		auto& info = pair.second;
		auto& localVarInfo = m_decompiler->m_localVars[localVar];
		localVarInfo.m_used = true;

		// try to change the size of the local var
		if (info.areAllParentOpNode) {
			ExprBitMask mask;
			for (auto& pair : info.m_opNodes) {
				auto opNode = pair.first;
				auto isSymbolInLeftNode = pair.second;
				auto anotherOperand = (isSymbolInLeftNode ? opNode->m_rightNode : opNode->m_leftNode);
				mask = mask | CalculateMask(anotherOperand);
			}

			// change the size of local var
			auto newLocalVarSize = mask.getSize();
			if (newLocalVarSize != 0 && newLocalVarSize != localVar->getSize()) {
				localVar->setSize(newLocalVarSize);
				localVarInfo.m_register.m_valueRangeMask = BitMask64(newLocalVarSize);
				// optimize all parent operational AND nodes
				for (auto& pair : info.m_opNodes) {
					auto opNode = pair.first;
					auto topNode = TopNode(opNode);
					ExprOptimization exprOptimization(&topNode);
					exprOptimization.start();
				}
			}
		}

		// iterate over all ctxs and create assignments: localVar1 = 0x5
		for (auto execCtx : localVarInfo.m_execCtxs) {
			auto expr = execCtx->m_registerExecCtx.requestRegister(localVarInfo.m_register);

			// to avoide: localVar1 = localVar1
			if (auto symbolLeaf = dynamic_cast<ExprTree::SymbolLeaf*>(expr))
				if (symbolLeaf->m_symbol == localVar)
					continue;

			// associate localVar with PCode instructions
			if (auto nodeRelToInstr = dynamic_cast<PCode::IRelatedToInstruction*>(expr)) {
				for (auto instr : nodeRelToInstr->getInstructionsRelatedTo())
					localVar->m_instructionsRelatedTo.push_back(instr);
			}

			// create assignment: localVar = {expr}
			auto& blockInfo = m_decompiler->m_decompiledBlocks[execCtx->m_pcodeBlock];
			blockInfo.m_decBlock->addSymbolParallelAssignmentLine(new ExprTree::SymbolLeaf(localVar), expr);
		}

		m_decGraph->addSymbol(localVar);
	}
}

void CE::Decompiler::Optimization::GraphParAssignmentCreator::optimizeAllParAssignments() {
	for (const auto decBlock : m_decGraph->getDecompiledBlocks()) {
		for (auto parAssignmentLine : decBlock->getSymbolParallelAssignmentLines()) {
			auto assignmentNode = parAssignmentLine->getAssignmentNode();

			// clone and optimize expr
			auto clonedExpr = assignmentNode->getSrcNode()->clone();
			auto topNode = TopNode(clonedExpr);
			ExprOptimization exprOptimization(&topNode);
			exprOptimization.start();

			assignmentNode->setSrcNode(topNode.getNode());
		}
	}
}
