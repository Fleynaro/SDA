#pragma once
#include "../ExprTree/ExprTree.h"
#include "../DecTopNode.h"

namespace CE::Decompiler
{
	class PCodeBlock;
};

namespace CE::Decompiler
{
	// Decompiled block which contains the abstract tree
	class DecBlock
	{
	public:
		// here are top nodes which are roots of expression trees

		// top node for a decompiled block
		class BlockTopNode : public TopNode
		{
		public:
			DecBlock* m_block;

			BlockTopNode(DecBlock* block, ExprTree::INode* node = nullptr)
				: m_block(block), TopNode(node)
			{}
		};

		// condition for jumping to another block
		class JumpTopNode : public BlockTopNode
		{
		public:
			JumpTopNode(DecBlock* block)
				: BlockTopNode(block)
			{}

			ExprTree::AbstractCondition* getCond() {
				return dynamic_cast<ExprTree::AbstractCondition*>(getNode());
			}

			void setCond(ExprTree::AbstractCondition* cond) {
				setNode(cond);
			}
		};

		// operator "return" which contains expr. tree that is the result of a function
		class ReturnTopNode : public BlockTopNode
		{
		public:
			ReturnTopNode(DecBlock* block)
				: BlockTopNode(block)
			{}
		};

		// line in high-level present (like code lines in C++), it may be function call(this is always assignment!) or normal assignment(e.g. localVar = 5)
		class SeqAssignmentLine : public BlockTopNode // consequent
		{
		public:
			SeqAssignmentLine(DecBlock* block, ExprTree::AssignmentNode* assignmentNode)
				: BlockTopNode(block, assignmentNode)
			{}

			SeqAssignmentLine(DecBlock* block, ExprTree::INode* dstNode, ExprTree::INode* srcNode, PCode::Instruction* instr)
				: SeqAssignmentLine(block, new ExprTree::AssignmentNode(dstNode, srcNode, instr))
			{}

			~SeqAssignmentLine() {
				m_block->m_seqLines.remove(this);
			}

			ExprTree::AssignmentNode* getAssignmentNode() {
				return dynamic_cast<ExprTree::AssignmentNode*>(getNode());
			}

			// left node from =
			ExprTree::INode* getDstNode() {
				return getAssignmentNode()->getDstNode();
			}

			// right node from =
			ExprTree::INode* getSrcNode() {
				return getAssignmentNode()->getSrcNode();
			}

			SeqAssignmentLine* clone(DecBlock* block, ExprTree::NodeCloneContext* ctx) {
				return new SeqAssignmentLine(block, dynamic_cast<ExprTree::AssignmentNode*>(getNode()->clone(ctx)));
			}
		};

		// temp line because it will be removed during graph optimization (lines expanding when parallel -> sequance)
		class SymbolParallelAssignmentLine : public SeqAssignmentLine // parallel
		{
		public:
			SymbolParallelAssignmentLine(DecBlock* block, ExprTree::AssignmentNode* assignmentNode)
				: SeqAssignmentLine(block, assignmentNode)
			{}

			SymbolParallelAssignmentLine(DecBlock* block, ExprTree::SymbolLeaf* dstNode, ExprTree::INode* srcNode, PCode::Instruction* instr)
				: SeqAssignmentLine(block, dstNode, srcNode, instr)
			{}

			~SymbolParallelAssignmentLine() {
				m_block->m_symbolParallelAssignmentLines.remove(this);
			}

			ExprTree::SymbolLeaf* getDstSymbolLeaf() {
				return dynamic_cast<ExprTree::SymbolLeaf*>(getDstNode());
			}

			SymbolParallelAssignmentLine* clone(DecBlock* block, ExprTree::NodeCloneContext* ctx) {
				return new SymbolParallelAssignmentLine(block, dynamic_cast<ExprTree::AssignmentNode*>(getNode()->clone(ctx)));
			}
		};

	public:
		std::string m_name;
		int m_level = 0;
		PCodeBlock* m_pcodeBlock;
	private:
		std::list<DecBlock*> m_blocksReferencedTo;
		DecBlock* m_nextNearBlock = nullptr;
		DecBlock* m_nextFarBlock = nullptr;
		std::list<SeqAssignmentLine*> m_seqLines;
		std::list<SymbolParallelAssignmentLine*> m_symbolParallelAssignmentLines; // temporary list, empty in the end of graph optimization
		JumpTopNode* m_noJmpCond;
	public:
		int m_maxHeight = 0;
		DecompiledCodeGraph* m_decompiledGraph;

		DecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level)
			: m_decompiledGraph(decompiledGraph), m_pcodeBlock(pcodeBlock), m_level(level)
		{
			m_noJmpCond = new JumpTopNode(this);
		}

		~DecBlock() {
			clearCode();

			for (auto line : m_symbolParallelAssignmentLines) {
				delete line;
			}

			for (auto refBlock : m_blocksReferencedTo) {
				if (refBlock->m_nextNearBlock == this)
					refBlock->m_nextNearBlock = refBlock->m_nextFarBlock;
				else if (refBlock->m_nextFarBlock == this)
					refBlock->m_nextFarBlock = nullptr;
			}

			delete m_noJmpCond;
			
			disconnect();
		}

		void clearCode() {
			for (auto line : m_seqLines) {
				delete line;
			}
			m_seqLines.clear();
			m_noJmpCond->clear();
		}

		// make the block independent from the decompiled graph
		void disconnect() {
			for (auto nextBlock : getNextBlocks()) {
				nextBlock->removeRefBlock(this);
			}
			m_nextNearBlock = m_nextFarBlock = nullptr;
		}

		void removeRefBlock(DecBlock* block) {
			m_blocksReferencedTo.remove(block);
		}

		void setNextNearBlock(DecBlock* nextBlock) {
			if (nextBlock) {
				nextBlock->removeRefBlock(m_nextNearBlock);
				nextBlock->m_blocksReferencedTo.push_back(this);
			}
			m_nextNearBlock = nextBlock;
		}

		void setNextFarBlock(DecBlock* nextBlock) {
			if (nextBlock) {
				nextBlock->removeRefBlock(m_nextFarBlock);
				nextBlock->m_blocksReferencedTo.push_back(this);
			}
			m_nextFarBlock = nextBlock;
		}

		DecBlock* getNextNearBlock() {
			return m_nextNearBlock;
		}

		DecBlock* getNextFarBlock() {
			return m_nextFarBlock;
		}

		std::list<DecBlock*>& getBlocksReferencedTo() {
			return m_blocksReferencedTo;
		}

		std::list<DecBlock*> getNextBlocks() {
			std::list<DecBlock*> nextBlocks;
			if (m_nextFarBlock) {
				nextBlocks.push_back(m_nextFarBlock);
			}
			if (m_nextNearBlock) {
				nextBlocks.push_back(m_nextNearBlock);
			}
			return nextBlocks;
		}

		DecBlock* getNextBlock() {
			if (m_nextFarBlock) {
				return m_nextFarBlock;
			}
			if (m_nextNearBlock) {
				return m_nextNearBlock;
			}
			return nullptr;
		}

		void swapNextBlocks() {
			std::swap(m_nextNearBlock, m_nextFarBlock);
		}

		bool isCondition() {
			return m_nextNearBlock != nullptr && m_nextFarBlock != nullptr;
		}

		bool isCycle() {
			return (int)m_blocksReferencedTo.size() != getRefHighBlocksCount();
		}

		// get count of blocks which reference to this block
		int getRefBlocksCount() {
			return (int)m_blocksReferencedTo.size();
		}

		// get count of blocks which reference to this block without loops
		int getRefHighBlocksCount() {
			int count = 0;
			for (auto refBlock : m_blocksReferencedTo) {
				if (refBlock->m_level < m_level)
					count++;
			}
			return count;
		}

		// get all top nodes for this block (assignments, function calls, return) / get all expressions
		virtual std::list<BlockTopNode*> getAllTopNodes() {
			std::list<BlockTopNode*> result;
			for (auto line : getSeqAssignmentLines()) {
				result.push_back(line);
			}
			for (auto line : getSymbolParallelAssignmentLines()) {
				result.push_back(line);
			}

			if(getNoJumpCondition())
				result.push_back(m_noJmpCond);
			return result;
		}

		// condition top node which contains boolean expression to jump to another block
		ExprTree::AbstractCondition* getNoJumpCondition() {
			return m_noJmpCond->getCond();
		}

		void setNoJumpCondition(ExprTree::AbstractCondition* noJmpCond) {
			if (getNoJumpCondition()) {
				m_noJmpCond->clear();
			}
			if (noJmpCond) {
				m_noJmpCond->setNode(noJmpCond);
			}
		}

		void addSeqLine(ExprTree::INode* destAddr, ExprTree::INode* srcValue, PCode::Instruction* instr = nullptr) {
			m_seqLines.push_back(new SeqAssignmentLine(this, destAddr, srcValue, instr));
		}

		std::list<SeqAssignmentLine*>& getSeqAssignmentLines() {
			return m_seqLines;
		}

		void addSymbolParallelAssignmentLine(ExprTree::SymbolLeaf* symbolLeaf, ExprTree::INode* srcValue, PCode::Instruction* instr = nullptr) {
			m_symbolParallelAssignmentLines.push_back(new SymbolParallelAssignmentLine(this, symbolLeaf, srcValue, instr));
		}
		
		std::list<SymbolParallelAssignmentLine*>& getSymbolParallelAssignmentLines() {
			return m_symbolParallelAssignmentLines;
		}

		// check if this block is empty
		bool hasNoCode() {
			return m_seqLines.empty() && m_symbolParallelAssignmentLines.empty();
		}

		// clone all expr.
		virtual void cloneAllExpr() {
			ExprTree::NodeCloneContext nodeCloneContext;

			auto seqLines = m_seqLines;
			m_seqLines.clear();
			for (auto line : seqLines) {
				m_seqLines.push_back(line->clone(this, &nodeCloneContext));
				delete line;
			}

			auto symbolParallelAssignmentLines = m_symbolParallelAssignmentLines;
			m_symbolParallelAssignmentLines.clear();
			for (auto line : symbolParallelAssignmentLines) {
				m_symbolParallelAssignmentLines.push_back(line->clone(this, &nodeCloneContext));
				delete line;
			}

			if (getNoJumpCondition())
				setNoJumpCondition(dynamic_cast<ExprTree::AbstractCondition*>(getNoJumpCondition()->clone(&nodeCloneContext)));
		}

		std::string printDebug(bool cond = true, const std::string& tabStr = "") {
			std::string result;
			for (auto line : m_seqLines) {
				result += tabStr + line->getNode()->printDebug();
			}
			if(!m_symbolParallelAssignmentLines.empty())
				result += tabStr + "<Symbol assignments>:\n";
			for (auto line : m_symbolParallelAssignmentLines) {
				result += tabStr + "- " + line->getNode()->printDebug();
			}
			if (cond && getNoJumpCondition() != nullptr) {
				result += "------> Condition: " + getNoJumpCondition()->printDebug() + "\n";
			}
			return result;
		}
	};

	// Block in which the control flow end (the last instruction of these blocks is RET).
	class EndDecBlock : public DecBlock
	{
		ReturnTopNode* m_returnNode = nullptr; // operator return where the result is
	public:
		EndDecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level)
			: DecBlock(decompiledGraph, pcodeBlock, level)
		{
			m_returnNode = new ReturnTopNode(this);
		}

		~EndDecBlock() {
			delete m_returnNode;
		}

		std::list<BlockTopNode*> getAllTopNodes() override {
			auto list = DecBlock::getAllTopNodes();
			if (getReturnNode()) {
				list.push_back(m_returnNode);
			}
			return list;
		}

		ExprTree::INode* getReturnNode() {
			return m_returnNode->getNode();
		}

		void setReturnNode(ExprTree::INode* returnNode) {
			if (getReturnNode()) {
				m_returnNode->clear();
			}
			m_returnNode->setNode(returnNode);
		}

		void cloneAllExpr() override {
			DecBlock::cloneAllExpr();
			if (getReturnNode())
				setReturnNode(getReturnNode()->clone());
		}
	};
};
