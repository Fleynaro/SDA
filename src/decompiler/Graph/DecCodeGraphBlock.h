#pragma once
#include "DecPCodeGraph.h"
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
		friend class CodeViewGenerator;
	public:
		// here are top nodes which are roots of expression trees

		// top node for a decompiled block
		class BlockTopNode : public TopNode
		{
		public:
			DecBlock* m_block;

			BlockTopNode(DecBlock* block, ExprTree::INode* node = nullptr);

			virtual Instruction* getLastReqInstr();
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
		class AssignmentLine : public BlockTopNode // consequent
		{
		public:
			Instruction* m_lastRequiredInstruction = nullptr;
			
			AssignmentLine(DecBlock* block, ExprTree::AssignmentNode* assignmentNode);

			AssignmentLine(DecBlock* block, ExprTree::INode* dstNode, ExprTree::INode* srcNode, PCode::Instruction* instr);

			~AssignmentLine();

			ExprTree::AssignmentNode* getAssignmentNode();

			// left node from =
			ExprTree::INode* getDstNode();

			// right node from =
			ExprTree::INode* getSrcNode();

			Instruction* getLastReqInstr() override;

			AssignmentLine* clone(DecBlock* block, ExprTree::NodeCloneContext* ctx);
		};

		std::string m_name;
		int m_level = 0;
		PCodeBlock* m_pcodeBlock;
	private:
		std::list<DecBlock*> m_blocksReferencedTo;
		DecBlock* m_nextNearBlock = nullptr;
		DecBlock* m_nextFarBlock = nullptr;
		std::list<AssignmentLine*> m_seqLines;
		std::list<AssignmentLine*> m_symbolParallelAssignmentLines; // temporary list, empty in the end of graph optimization
		JumpTopNode* m_noJmpCond;
	public:
		std::list<DecBlock*> m_joinedRemovedBlocks;
		int m_maxHeight = 0; // calculated as count of the lines from the top to the block
		DecompiledCodeGraph* m_decompiledGraph;

		DecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level);

		~DecBlock();

		void clearCode();

		// make the block independent from the decompiled graph
		void disconnect();

		void removeRefBlock(DecBlock* block);

		void setNextNearBlock(DecBlock* nextBlock);

		void setNextFarBlock(DecBlock* nextBlock);

		DecBlock* getNextNearBlock() const;

		DecBlock* getNextFarBlock() const;

		std::list<DecBlock*>& getBlocksReferencedTo();

		std::list<DecBlock*> getNextBlocks() const;

		DecBlock* getNextBlock() const;

		void swapNextBlocks();

		bool isCondition() const;

		bool isCycle();

		// get count of blocks which reference to this block
		int getRefBlocksCount() const;

		// get count of blocks which reference to this block without loops
		int getRefHighBlocksCount();

		// get all top nodes for this block (assignments, function calls, return) / get all expressions
		virtual std::list<BlockTopNode*> getAllTopNodes();

		JumpTopNode* getJumpTopNode() const;

		// condition top node which contains boolean expression to jump to another block
		ExprTree::AbstractCondition* getNoJumpCondition() const;

		void setNoJumpCondition(ExprTree::AbstractCondition* noJmpCond) const;

		AssignmentLine* addSeqLine(ExprTree::INode* destAddr, ExprTree::INode* srcValue, PCode::Instruction* instr = nullptr);

		std::list<AssignmentLine*>& getSeqAssignmentLines();

		AssignmentLine* addSymbolParallelAssignmentLine(ExprTree::SymbolLeaf* symbolLeaf, ExprTree::INode* srcValue, PCode::Instruction* instr = nullptr);
		
		std::list<AssignmentLine*>& getSymbolParallelAssignmentLines();

		BlockTopNode* findBlockTopNodeByOffset(ComplexOffset offset);

		// check if this block is empty
		bool hasNoCode() const;

		// clone all expr.
		virtual void cloneAllExpr();
	};

	// Block in which the control flow end (the last instruction of these blocks is RET).
	class EndDecBlock : public DecBlock
	{
		ReturnTopNode* m_returnNode = nullptr; // operator return where the result is
	public:
		EndDecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level);

		~EndDecBlock();

		std::list<BlockTopNode*> getAllTopNodes() override;

		ReturnTopNode* getReturnTopNode() const;

		void setReturnNode(ExprTree::INode* returnNode) const;

		void cloneAllExpr() override;
	};
};
