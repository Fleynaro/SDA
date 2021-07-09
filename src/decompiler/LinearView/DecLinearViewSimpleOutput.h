#pragma once
#include "DecLinearView.h"

namespace CE::Decompiler
{
	class LinearViewSimpleOutput
	{
		LinearView::BlockList* m_blockList;
		DecompiledCodeGraph* m_decGraph;
		std::string m_textCode;
	public:
		bool m_SHOW_ASM = true;
		bool m_SHOW_PCODE = false;
		bool m_SHOW_ALL_GOTO = true;
		bool m_SHOW_LINEAR_LEVEL_EXT = true;
		bool m_SHOW_BLOCK_HEADER = true;

		LinearViewSimpleOutput(LinearView::BlockList* blockList, DecompiledCodeGraph* decGraph)
			: m_blockList(blockList), m_decGraph(decGraph)
		{}

		std::string getTextCode() const
		{
			return m_textCode;
		}

		void setMinInfoToShow() {
			m_SHOW_ASM = false;
			m_SHOW_PCODE = false;
			m_SHOW_ALL_GOTO = false;
			m_SHOW_LINEAR_LEVEL_EXT = false;
		}

		void generate() {
			genCode(m_blockList);
		}

		void show() {
			generate();
			printf("%s", m_textCode.c_str());
		}
	private:
		std::set<LinearView::Block*> m_blocksToGoTo;

		void genCode(LinearView::BlockList* blockList, int level = 0) {
			std::string tabStr = "";
			for (int i = 0; i < level; i++)
				tabStr += "\t";

			for (auto block : blockList->getBlocks()) {
				const auto decBlock = block->m_decBlock;
				const auto asmBlock = decBlock->m_pcodeBlock;

				if (auto condition = dynamic_cast<LinearView::Condition*>(block)) {
					showBlockCode(asmBlock, block, tabStr);
					m_textCode += tabStr + "if("+ (condition->m_cond ? condition->m_cond->printDebug() : "") +") {\n";
					genCode(condition->m_mainBranch, level + 1);
					if (m_SHOW_ALL_GOTO || !condition->m_elseBranch->isEmpty()) {
						m_textCode += tabStr + "} else {\n";
						genCode(condition->m_elseBranch, level + 1);
					}
					m_textCode += tabStr +"}\n";
				}
				else if (auto whileCycle = dynamic_cast<LinearView::WhileCycle*>(block)) {
					if (!whileCycle->m_isDoWhileCycle) {
						showBlockCode(asmBlock, block, tabStr);
						m_textCode += tabStr + "while("+ (whileCycle->m_cond ? whileCycle->m_cond->printDebug() : "") +") {\n";
						genCode(whileCycle->m_mainBranch, level + 1);
						m_textCode += tabStr +"}\n";
					}
					else {
						m_textCode += tabStr +"do {\n";
						genCode(whileCycle->m_mainBranch, level + 1);
						showBlockCode(asmBlock, block, "\t" + tabStr);
						m_textCode += tabStr +"} while("+ (whileCycle->m_cond ? whileCycle->m_cond->printDebug() : "") +");\n";
					}
				}
				else {
					showBlockCode(asmBlock, block, tabStr);
				}

				if (const auto endBlock = dynamic_cast<EndDecBlock*>(decBlock)) {
					if (endBlock->getReturnNode() != nullptr) {
						m_textCode += tabStr +"return "+ endBlock->getReturnNode()->printDebug() +"\n";
					}
				}
			}

			std::string levelInfo;
			if (m_SHOW_LINEAR_LEVEL_EXT) {
				levelInfo = "backOrderId: " + std::to_string(blockList->getBackOrderId()) + "; minLinLevel: " + std::to_string(blockList->getMinLinearLevel()) + ", maxLinLevel: " + std::to_string(blockList->getMaxLinearLevel()) + "";
			}

			if (blockList->m_goto != nullptr) {
				const auto gotoType = blockList->getGotoType();
				if (m_SHOW_ALL_GOTO || gotoType != LinearView::GotoType::None) {
					const auto blockName = Helper::String::NumberToHex(blockList->m_goto->m_decBlock->m_pcodeBlock->ID);
					std::string typeName = "";
					if (gotoType == LinearView::GotoType::None)
						typeName = "[None]";
					else if (gotoType == LinearView::GotoType::Normal) {
						typeName = "[Goto to label_" + blockName + "]";
						m_blocksToGoTo.insert(blockList->m_goto);
					}
					else if (gotoType == LinearView::GotoType::Break)
						typeName = "[break]";
					else if (gotoType == LinearView::GotoType::Continue)
						typeName = "[continue]";
					if (m_SHOW_ALL_GOTO) {
						m_textCode += tabStr + "//goto to block "+ blockName +" ("+ levelInfo +") "+ typeName +"\n";
					}
					else {
						m_textCode += tabStr + typeName +"\n";
					}
				}
			}
			else if (m_SHOW_ALL_GOTO) {
				m_textCode += tabStr +"//goto is null ("+ levelInfo +")\n";
			}
		}

		void showBlockCode(PCodeBlock* pcodeBlock, LinearView::Block* block, std::string tabStr) {
			const auto blockName = Helper::String::NumberToHex(pcodeBlock->ID);
			if (m_SHOW_BLOCK_HEADER) {
				m_textCode += tabStr +"//block "+ blockName +" (level: "+ std::to_string(block->m_decBlock->m_level) +", maxHeight: "+ std::to_string(block->m_decBlock->m_maxHeight) +", backOrderId: " + std::to_string(block->getBackOrderId()) + ", linearLevel: " + std::to_string(block->getLinearLevel()) + ", refCount: " + std::to_string(block->m_decBlock->getRefBlocksCount()) + ")\n";
			}
			if (m_blocksToGoTo.find(block) != m_blocksToGoTo.end()) {
				m_textCode += tabStr + "label_"+ blockName +":\n";
			}
			if (m_SHOW_ASM) {
				m_textCode += pcodeBlock->printDebug(nullptr, tabStr, false, m_SHOW_PCODE);
				m_textCode += tabStr +"------------\n";
			}
			m_textCode += block->m_decBlock->printDebug(false, tabStr);
		}
	};
};