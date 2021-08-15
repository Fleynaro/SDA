#pragma once
#include <decompiler/Graph/DecCodeGraph.h>

namespace CE::Decompiler::LinearView
{
	enum class GotoType {
		None,
		Normal,
		Continue,
		Break
	};

	class BlockList;
	class WhileCycleBlock;

	// code, condition or while cycle
	class AbstractBlock
	{
	public:
		int m_backOrderId = 0;
		int m_linearLevel = 0;
		BlockList* m_blockList = nullptr;

		virtual ~AbstractBlock() {}

		virtual std::list<BlockList*> getBlockLists() = 0;

		virtual WhileCycleBlock* getWhileCycle();
	};

	// always associated with dec. block
	class CodeBlock : public AbstractBlock
	{
	public:
		DecBlock* m_decBlock; //todo: remove for while
		std::list<BlockList*> m_refBlockLists;

		CodeBlock(DecBlock* decBlock)
			: m_decBlock(decBlock)
		{}

		std::list<BlockList*> getBlockLists() override {
			return {};
		}
	};

	class BlockList
	{
		std::list<AbstractBlock*> m_blocks;
	public:
		AbstractBlock* m_parentBlock; // condition or while
		CodeBlock* m_goto = nullptr;
		int m_backOrderId = 0;
		int m_minLinearLevel = 0;
		int m_maxLinearLevel = 0;

		BlockList(AbstractBlock* parent = nullptr)
			: m_parentBlock(parent)
		{}

		void setGoto(CodeBlock* block) {
			if (m_goto) {
				m_goto->m_refBlockLists.remove(this);
			}
			block->m_refBlockLists.push_back(this);
			m_goto = block;
		}

		void addBlock(AbstractBlock* block) {
			block->m_blockList = this;
			m_blocks.push_back(block);
		}

		void removeBlock(AbstractBlock* block) {
			block->m_blockList = nullptr;
			m_blocks.remove(block);
		}

		virtual std::list<AbstractBlock*> getBlocks() {
			return m_blocks;
		}

		AbstractBlock* getFirstBlock() const {
			if (m_blocks.empty())
				return nullptr;
			return *m_blocks.begin();
		}

		CodeBlock* findBlock(DecBlock* decBlock);

		bool hasGoto() const;

		GotoType getGotoType();

		WhileCycleBlock* getWhileCycle() const;

		bool isEmpty() {
			return m_blocks.empty() && getGotoType() == GotoType::None;
		}
	};

	class ConditionBlock : public AbstractBlock
	{
	public:
		BlockList* m_mainBranch;
		BlockList* m_elseBranch;
		ExprTree::AbstractCondition* m_cond;
		DecBlock::JumpTopNode* m_jmpTopNode;

		ConditionBlock(DecBlock::JumpTopNode* jmpTopNode)
			: m_jmpTopNode(jmpTopNode)
		{
			m_mainBranch = new BlockList(this);
			m_elseBranch = new BlockList(this);
			m_cond = dynamic_cast<ExprTree::AbstractCondition*>(m_jmpTopNode->getCond()->clone());
		}

		~ConditionBlock() {
			delete m_mainBranch;
			delete m_elseBranch;
			delete m_cond;
		}

		std::list<BlockList*> getBlockLists() override {
			return { m_mainBranch, m_elseBranch };
		}

		void inverse() {
			m_cond->inverse();
			std::swap(m_mainBranch, m_elseBranch);
		}
	};

	class WhileCycleBlock : public AbstractBlock
	{
	public:
		BlockList* m_mainBranch;
		ExprTree::AbstractCondition* m_cond;
		DecBlock::JumpTopNode* m_jmpTopNode;
		bool m_isDoWhileCycle;

		WhileCycleBlock(DecBlock::JumpTopNode* jmpTopNode, bool isDoWhileCycle = false)
			: m_jmpTopNode(jmpTopNode), m_isDoWhileCycle(isDoWhileCycle)
		{
			m_mainBranch = new BlockList(this);
			if(m_jmpTopNode && m_jmpTopNode->getCond()) {
				m_cond = dynamic_cast<ExprTree::AbstractCondition*>(m_jmpTopNode->getCond()->clone());
				if (isDoWhileCycle)
					m_cond->inverse();
			} else {
				m_cond = new ExprTree::BooleanValue(true);
			}
		}

		~WhileCycleBlock() {
			delete m_mainBranch;
			delete m_cond;
		}

		AbstractBlock* getFirstBlock() const {
			return *m_mainBranch->getBlocks().begin();
		}

		std::list<BlockList*> getBlockLists() override {
			return { m_mainBranch };
		}

		WhileCycleBlock* getWhileCycle() override {
			return this;
		}
	};
};