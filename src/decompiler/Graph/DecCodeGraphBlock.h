#pragma once
#include <decompiler/DecTopNode.h>
#include <decompiler/ExprTree/ExprTree.h>
#include <list>

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

			BlockTopNode(DecBlock* block, ExprTree::INode* node = nullptr);
		};

		// condition for jumping to another block
		class JumpTopNode : public BlockTopNode
		{
		public:
			JumpTopNode(DecBlock* block);

			ExprTree::AbstractCondition* getCond();

			void setCond(ExprTree::AbstractCondition* cond);
		};

		// operator "return" which contains expr. tree that is the result of a function
		class ReturnTopNode : public BlockTopNode
		{
		public:
			ReturnTopNode(DecBlock* block);
		};

		// line in high-level present (like code lines in C++), it may be function call(this is always assignment!) or normal assignment(e.g. localVar = 5)
		class SeqAssignmentLine : public BlockTopNode // consequent
		{
		public:
			SeqAssignmentLine(DecBlock* block, ExprTree::AssignmentNode* assignmentNode);

			SeqAssignmentLine(DecBlock* block, ExprTree::INode* dstNode, ExprTree::INode* srcNode, PCode::Instruction* instr);

			~SeqAssignmentLine();

			ExprTree::AssignmentNode* getAssignmentNode();

			// left node from =
			ExprTree::INode* getDstNode();

			// right node from =
			ExprTree::INode* getSrcNode();

			SeqAssignmentLine* clone(DecBlock* block, ExprTree::NodeCloneContext* ctx);
		};

		// temp line because it will be removed during graph optimization (lines expanding when parallel -> sequance)
		class SymbolParallelAssignmentLine : public SeqAssignmentLine // parallel
		{
		public:
			SymbolParallelAssignmentLine(DecBlock* block, ExprTree::AssignmentNode* assignmentNode);

			SymbolParallelAssignmentLine(DecBlock* block, ExprTree::SymbolLeaf* dstNode, ExprTree::INode* srcNode, PCode::Instruction* instr);

			~SymbolParallelAssignmentLine();

			ExprTree::SymbolLeaf* getDstSymbolLeaf();

			SymbolParallelAssignmentLine* clone(DecBlock* block, ExprTree::NodeCloneContext* ctx);
		};

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

		DecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level);

		~DecBlock();

		void clearCode();

		// make the block independent from the decompiled graph
		void disconnect();

		void removeRefBlock(DecBlock* block);

		void setNextNearBlock(DecBlock* nextBlock);

		void setNextFarBlock(DecBlock* nextBlock);

		DecBlock* getNextNearBlock();

		DecBlock* getNextFarBlock();

		std::list<DecBlock*>& getBlocksReferencedTo();

		std::list<DecBlock*> getNextBlocks();

		DecBlock* getNextBlock();

		void swapNextBlocks();

		bool isCondition();

		bool isCycle();

		// get count of blocks which reference to this block
		int getRefBlocksCount();

		// get count of blocks which reference to this block without loops
		int getRefHighBlocksCount();

		// get all top nodes for this block (assignments, function calls, return) / get all expressions
		virtual std::list<BlockTopNode*> getAllTopNodes();

		// condition top node which contains boolean expression to jump to another block
		ExprTree::AbstractCondition* getNoJumpCondition();

		void setNoJumpCondition(ExprTree::AbstractCondition* noJmpCond);

		void addSeqLine(ExprTree::INode* destAddr, ExprTree::INode* srcValue, PCode::Instruction* instr = nullptr);

		std::list<SeqAssignmentLine*>& getSeqAssignmentLines();

		void addSymbolParallelAssignmentLine(ExprTree::SymbolLeaf* symbolLeaf, ExprTree::INode* srcValue, PCode::Instruction* instr = nullptr);
		
		std::list<SymbolParallelAssignmentLine*>& getSymbolParallelAssignmentLines();

		// check if this block is empty
		bool hasNoCode();

		// clone all expr.
		virtual void cloneAllExpr();

		std::string printDebug(bool cond = true, const std::string& tabStr = "");
	};

	// Block in which the control flow end (the last instruction of these blocks is RET).
	class EndDecBlock : public DecBlock
	{
		ReturnTopNode* m_returnNode = nullptr; // operator return where the result is
	public:
		EndDecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level);

		~EndDecBlock();

		std::list<BlockTopNode*> getAllTopNodes() override;

		ExprTree::INode* getReturnNode();

		void setReturnNode(ExprTree::INode* returnNode);

		void cloneAllExpr() override;
	};
};
