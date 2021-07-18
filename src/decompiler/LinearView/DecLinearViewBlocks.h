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

	// condition or cycle block
	class IBlockListAgregator
	{
	public:
		virtual std::list<BlockList*> getBlockLists() = 0;
	};

	class Block
	{
	public:
		int m_backOrderId = 0;
		int m_linearLevel = 0;
		DecBlock* m_decBlock; //todo: remove for while
		BlockList* m_blockList = nullptr;
		std::list<BlockList*> m_refBlockLists;

		Block(DecBlock* decBlock)
			: m_decBlock(decBlock)
		{}

		virtual ~Block() {}

		int getBackOrderId() const
		{
			return m_backOrderId;
		}

		int getLinearLevel() const
		{
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

		virtual std::list<Block*> getBlocks() {
			return m_blocks;
		}

		Block* findBlock(DecBlock* decBlock);

		bool hasGoto();

		GotoType getGotoType();

		WhileCycle* getWhileCycle() const;

		bool isEmpty() {
			return getBlocks().size() == 0 && getGotoType() == GotoType::None;
		}

		int getBackOrderId() const
		{
			return m_backOrderId;
		}

		int getMinLinearLevel() const
		{
			return m_minLinearLevel;
		}

		int getMaxLinearLevel() const
		{
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
		ExprTree::AbstractCondition* m_cond = nullptr;

		Condition(DecBlock* decBlock)
			: Block(decBlock)
		{
			m_mainBranch = new BlockList(this);
			m_elseBranch = new BlockList(this);
			createCondition();
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

	private:
		void createCondition() {
			if (m_decBlock->getNoJumpCondition())
				m_cond = dynamic_cast<ExprTree::AbstractCondition*>(m_decBlock->getNoJumpCondition()->clone());
		}
	};

	class WhileCycle : public Block, public IBlockListAgregator
	{
		// do-while cycles is not associated with dec. block, its dec. block is in its block list in the end
		/*class DoWhileBlockList : public BlockList
		{
			Block* m_block;
		public:
			DoWhileBlockList(IBlockListAgregator* parent, DecBlock* decBlock)
				: BlockList(parent), m_block(new Block(decBlock))
			{}

			~DoWhileBlockList() {
				delete m_block;
			}

			std::list<Block*> getBlocks() override {
				auto blocks = BlockList::getBlocks();
				blocks.push_back(m_block);
				return blocks;
			}
		};*/
	public:
		BlockList* m_mainBranch;
		ExprTree::AbstractCondition* m_cond;
		bool m_isDoWhileCycle;
		bool m_isInfinite;

		WhileCycle(DecBlock* decBlock, bool isDoWhileCycle = false, bool isInfinite = false)
			: Block(isDoWhileCycle ? nullptr : decBlock), m_isDoWhileCycle(isDoWhileCycle), m_isInfinite(isInfinite)
		{
			m_mainBranch = new BlockList(this);
			createCondition(decBlock);
		}

		~WhileCycle() {
			delete m_mainBranch;
			delete m_cond;
		}

		Block* getFirstBlock() const {
			return *m_mainBranch->getBlocks().begin();
		}

		std::list<BlockList*> getBlockLists() override {
			return { m_mainBranch };
		}

		WhileCycle* getWhileCycle() override {
			return this;
		}

	private:
		void createCondition(DecBlock* decBlock) {
			if (m_isInfinite) {
				m_cond = new ExprTree::BooleanValue(true);
			}
			else {
				m_cond = dynamic_cast<ExprTree::AbstractCondition*>(decBlock->getNoJumpCondition()->clone());
				if (m_isDoWhileCycle) {
					m_cond->inverse();
				}
			}
		}
	};
};