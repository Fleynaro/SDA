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
	class WhileCycle;

	class IBlockListAgregator
	{
	public:
		virtual std::list<BlockList*> getBlockLists() = 0;

		virtual bool isInversed() {
			return false;
		}
	};

	class Block
	{
	public:
		int m_backOrderId = 0;
		int m_linearLevel = 0;
		DecBlock* m_decBlock;
		BlockList* m_blockList = nullptr;
		std::list<BlockList*> m_refBlockLists;

		Block(DecBlock* decBlock)
			: m_decBlock(decBlock)
		{}

		virtual ~Block() {}

		int getBackOrderId() {
			return m_backOrderId;
		}

		int getLinearLevel() {
			return m_linearLevel;
		}

		std::list<BlockList*>& getRefBlockLists() {
			return m_refBlockLists;
		}

		virtual WhileCycle* getWhileCycle();
	};

	class BlockList
	{
	public:
		int m_backOrderId = 0;
		int m_minLinearLevel = 0;
		int m_maxLinearLevel = 0;
		IBlockListAgregator* m_parent;
		Block* m_goto = nullptr;

		BlockList(IBlockListAgregator* parent = nullptr)
			: m_parent(parent)
		{}

		void setGoto(Block* block) {
			if (m_goto) {
				m_goto->getRefBlockLists().remove(this);
			}
			block->getRefBlockLists().push_back(this);
			m_goto = block;
		}

		void addBlock(Block* block) {
			block->m_blockList = this;
			m_blocks.push_back(block);
		}

		void removeBlock(Block* block) {
			block->m_blockList = nullptr;
			m_blocks.remove(block);
		}

		std::list<Block*>& getBlocks() {
			return m_blocks;
		}

		Block* findBlock(DecBlock* decBlock);

		bool hasGoto();

		GotoType getGotoType();

		WhileCycle* getWhileCycle();

		bool isEmpty() {
			return getBlocks().size() == 0 && getGotoType() == GotoType::None;
		}

		int getBackOrderId() {
			return m_backOrderId;
		}

		int getMinLinearLevel() {
			return m_minLinearLevel;
		}

		int getMaxLinearLevel() {
			return m_maxLinearLevel;
		}
	private:
		std::list<Block*> m_blocks;
	};

	class Condition : public Block, public IBlockListAgregator
	{
	public:
		BlockList* m_mainBranch;
		BlockList* m_elseBranch;
		ExprTree::AbstractCondition* m_cond;

		Condition(DecBlock* decBlock)
			: Block(decBlock)
		{
			m_mainBranch = new BlockList(this);
			m_elseBranch = new BlockList(this);
			m_cond = decBlock->getNoJumpCondition() ? dynamic_cast<ExprTree::AbstractCondition*>(decBlock->getNoJumpCondition()->clone()) : nullptr;
		}

		~Condition() {
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

	class WhileCycle : public Block, public IBlockListAgregator
	{
	public:
		BlockList* m_mainBranch;
		ExprTree::AbstractCondition* m_cond;
		bool m_isDoWhileCycle;
		bool m_isInfinite;

		WhileCycle(DecBlock* decBlock, bool isDoWhileCycle = false, bool isInfinite = false)
			: Block(decBlock), m_isDoWhileCycle(isDoWhileCycle), m_isInfinite(isInfinite)
		{
			m_mainBranch = new BlockList(this);
			if (m_isInfinite) {
				m_cond = new ExprTree::BooleanValue(true);
			}
			else {
				m_cond = dynamic_cast<ExprTree::AbstractCondition*>(decBlock->getNoJumpCondition()->clone());
				if (isDoWhileCycle) {
					m_cond->inverse();
				}
			}
		}

		~WhileCycle() {
			delete m_mainBranch;
			delete m_cond;
		}

		Block* getFirstBlock() {
			if (!m_isDoWhileCycle) {
				return this;
			}
			return *m_mainBranch->getBlocks().begin();
		}

		std::list<BlockList*> getBlockLists() override {
			return { m_mainBranch };
		}

		bool isInversed() override {
			return m_isDoWhileCycle;
		}

		WhileCycle* getWhileCycle() override {
			return this;
		}
	};
};