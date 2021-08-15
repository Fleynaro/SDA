#include "DecLinearView.h"

using namespace CE::Decompiler;
using namespace LinearView;

WhileCycleBlock* AbstractBlock::getWhileCycle() {
	return m_blockList->getWhileCycle();
}

WhileCycleBlock* BlockList::getWhileCycle() const {
	if (m_parentBlock) {
		return m_parentBlock->getWhileCycle();
	}
	return nullptr;
}

CodeBlock* BlockList::findBlock(DecBlock* decBlock) {
	for (const auto block : m_blocks) {
		if (const auto codeBlock = dynamic_cast<CodeBlock*>(block)) {
			if(decBlock == codeBlock->m_decBlock)
				return codeBlock;
		}
		
		for (const auto blockList : block->getBlockLists()) {
			const auto block = blockList->findBlock(decBlock);
			if (block != nullptr) {
				return block;
			}
		}
	}

	return nullptr;
}

bool BlockList::hasGoto() const {
	if (!m_goto)
		return false;
	if (m_goto->m_linearLevel >= m_maxLinearLevel) {
		if (m_goto->m_backOrderId == m_backOrderId)
			return false;
	}
	return true;
}

GotoType BlockList::getGotoType() {
	if (!m_goto)
		return GotoType::None;
	if (const auto whileCycle = getWhileCycle()) {
		if (m_goto->m_linearLevel >= m_maxLinearLevel) {
			if (m_goto->m_backOrderId == whileCycle->m_backOrderId - 1)
				return GotoType::Break;
		}
		else {
			if (m_goto == whileCycle->getFirstBlock()) {
				if(m_backOrderId == whileCycle->m_backOrderId - 1)
					return GotoType::None;
				return GotoType::Continue;
			}
		}
	}
	if (!hasGoto())
		return GotoType::None;
	return GotoType::Normal;
}