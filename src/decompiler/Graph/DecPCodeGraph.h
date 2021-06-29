#pragma once
#include "../PCode/DecPCode.h"
#include "../PCode/DecPCodeConstValueCalc.h"

namespace CE::Decompiler
{
	class ImagePCodeGraph;
	class FunctionPCodeGraph;

	// pcode graph for a non-branched block
	class PCodeBlock
	{
		int64_t m_minOffset;
		int64_t m_maxOffset;
		std::list<PCode::Instruction*> m_instructions; // content of the block
		PCodeBlock* m_nextNearBlock = nullptr;
		PCodeBlock* m_nextFarBlock = nullptr;
	public:
		int ID = 0;
		int m_level = 0;
		std::list<PCodeBlock*> m_blocksReferencedTo;
		FunctionPCodeGraph* m_funcPCodeGraph = nullptr;

		PCodeBlock() = default;

		PCodeBlock(int64_t minOffset, int64_t maxOffset)
			: m_minOffset(minOffset), m_maxOffset(maxOffset), ID((int)(minOffset >> 8))
		{}

		void removeRefBlock(PCodeBlock* block) {
			m_blocksReferencedTo.remove(block);
		}

		void disconnect() {
			for (auto nextBlock : getNextBlocks()) {
				nextBlock->removeRefBlock(this);
			}
			m_nextNearBlock = m_nextFarBlock = nullptr;
		}

		std::list<PCode::Instruction*>& getInstructions() {
			return m_instructions;
		}

		int64_t getMinOffset() {
			return m_minOffset;
		}

		int64_t getMaxOffset() { // todo: auto-calculated?
			return m_maxOffset;
		}

		void setMaxOffset(int64_t offset) {
			m_maxOffset = offset;
		}

		void removeNextBlock(PCodeBlock* nextBlock) {
			if (nextBlock == m_nextNearBlock)
				m_nextNearBlock = nullptr;
			if (nextBlock == m_nextFarBlock)
				m_nextFarBlock = nullptr;
		}

		void setNextNearBlock(PCodeBlock* nextBlock) {
			m_nextNearBlock = nextBlock;
			nextBlock->m_blocksReferencedTo.push_back(this);
		}

		void setNextFarBlock(PCodeBlock* nextBlock) {
			m_nextFarBlock = nextBlock;
			nextBlock->m_blocksReferencedTo.push_back(this);
		}

		PCodeBlock* getNextNearBlock() {
			return m_nextNearBlock;
		}

		PCodeBlock* getNextFarBlock() {
			return m_nextFarBlock;
		}

		std::list<PCodeBlock*> getNextBlocks() {
			std::list<PCodeBlock*> nextBlocks;
			if (m_nextFarBlock) {
				nextBlocks.push_back(m_nextFarBlock);
			}
			if (m_nextNearBlock) {
				nextBlocks.push_back(m_nextNearBlock);
			}
			return nextBlocks;
		}

		PCode::Instruction* getLastInstruction() {
			return *std::prev(m_instructions.end());
		}

		std::string printDebug(void* addr, const std::string& tabStr, bool extraInfo, bool pcode) {
			std::string result;

			ZyanU64 runtime_address = (ZyanU64)addr;
			for (auto instr : m_instructions) {
				std::string prefix = tabStr + "0x" + Helper::String::NumberToHex(runtime_address + instr->m_origInstruction->m_offset);
				if (!instr->m_origInstruction->m_originalView.empty())
					result += prefix + " " + instr->m_origInstruction->m_originalView + "\n";
				if (pcode) {
					prefix += ":" + std::to_string(instr->m_orderId) + "(" + Helper::String::NumberToHex(instr->getOffset()) + ")";
					result += "\t" + prefix + " " + instr->printDebug();
					if (instr->m_id == PCode::InstructionId::UNKNOWN)
						result += " <------------------------------------------------ ";
					result += "\n";
				}
			}

			if (extraInfo) {
				result += "Level: "+ std::to_string(m_level) +"\n";
				if (m_nextNearBlock != nullptr)
					result += "Next near: "+ Helper::String::NumberToHex(m_nextNearBlock->getMinOffset()) +"\n";
				if (m_nextFarBlock != nullptr)
					result += "Next far: " + Helper::String::NumberToHex(m_nextFarBlock->getMinOffset()) + "\n";
			}

			return result;
		}
	};

	// pcode graph (consisted of PCode connected blocks) for a function
	class FunctionPCodeGraph
	{
		ImagePCodeGraph* m_imagePCodeGraph;
		PCodeBlock* m_startBlock = nullptr;
		std::set<PCodeBlock*> m_blocks;
		std::set<FunctionPCodeGraph*> m_refFuncCalls;
		std::set<FunctionPCodeGraph*> m_nonVirtFuncCalls;
		std::set<FunctionPCodeGraph*> m_virtFuncCalls;
		std::map<PCode::Instruction*, DataValue> m_constValues;
	public:
		FunctionPCodeGraph(ImagePCodeGraph* imagePCodeGraph)
			: m_imagePCodeGraph(imagePCodeGraph)
		{}

		ImagePCodeGraph* getImagePCodeGraph() {
			return m_imagePCodeGraph;
		}

		void setStartBlock(PCodeBlock* block) {
			m_startBlock = block;
		}

		// head is a function that has not parents (main/all virtual functions)
		bool isHead() {
			return m_refFuncCalls.empty();
		}

		auto getRefFuncCalls() {
			return m_refFuncCalls;
		}

		auto getNonVirtFuncCalls() {
			return m_nonVirtFuncCalls;
		}

		auto getVirtFuncCalls() {
			return m_virtFuncCalls;
		}

		void addNonVirtFuncCall(FunctionPCodeGraph* funcGraph) {
			m_nonVirtFuncCalls.insert(funcGraph);
			funcGraph->m_refFuncCalls.insert(this);
		}

		void addVirtFuncCall(FunctionPCodeGraph* funcGraph) {
			m_virtFuncCalls.insert(funcGraph);
			funcGraph->m_refFuncCalls.insert(this);
		}

		const auto& getBlocks() {
			return m_blocks;
		}

		void addBlock(PCodeBlock* block) {
			m_blocks.insert(block);
			block->m_funcPCodeGraph = this;
		}

		PCodeBlock* getStartBlock() {
			return m_startBlock;
		}

		std::map<PCode::Instruction*, PCode::DataValue>& getConstValues() {
			return m_constValues;
		}

		void printDebug(void* addr) {
			std::list<PCodeBlock*> blocks;
			for (auto block : m_blocks)
				blocks.push_back(block);
			blocks.sort([](PCodeBlock* a, PCodeBlock* b) {
				return a->getMinOffset() < b->getMinOffset();
				});

			for (auto block : blocks) {
				puts(block->printDebug(addr, "", true, true).c_str());
				puts("==================");
			}
		}
	};

	// pcode graph (consisted of NON-connected function graphs in final state) for a whole program
	class ImagePCodeGraph
	{
		std::list<FunctionPCodeGraph> m_funcGraphList;
		std::list<FunctionPCodeGraph*> m_headFuncGraphs;
		std::map<int64_t, PCodeBlock> m_blocks;
	public:
		// exceptions
		class BlockNotFoundException : public std::exception {};

		ImagePCodeGraph()
		{}

		FunctionPCodeGraph* createFunctionGraph() {
			m_funcGraphList.push_back(FunctionPCodeGraph(this));
			return &*m_funcGraphList.rbegin();
		}

		PCodeBlock* createBlock(int64_t minOffset, int64_t maxOffset) {
			m_blocks.insert(std::make_pair(minOffset, PCodeBlock(minOffset, maxOffset)));
			auto newBlock = &m_blocks[minOffset];
			return newBlock;
		}

		PCodeBlock* createBlock(int64_t offset) {
			return createBlock(offset, offset);
		}

		const auto& getHeadFuncGraphs() {
			return m_headFuncGraphs;
		}

		auto& getFunctionGraphList() {
			return m_funcGraphList;
		}

		FunctionPCodeGraph* getEntryFunctionGraph() {
			return &*m_funcGraphList.begin();
		}

		PCodeBlock* getBlockAtOffset(int64_t offset, bool halfInterval = true) {
			auto it = std::prev(m_blocks.upper_bound(offset));
			if (it != m_blocks.end()) {
				bool boundUp = halfInterval ? (offset < it->second.getMaxOffset()) : (offset <= it->second.getMaxOffset());
				if (boundUp && offset >= it->second.getMinOffset()) {
					return &it->second;
				}
			}
			throw BlockNotFoundException();
		}

		FunctionPCodeGraph* getFuncGraphAt(int64_t offset, bool halfInterval = true) {
			auto block = getBlockAtOffset(offset, halfInterval);
			return block->m_funcPCodeGraph;
		}

		// add all head functions into the list HeadFuncGraphs
		void fillHeadFuncGraphs() {
			for (auto& funcGraph : getFunctionGraphList()) {
				if (funcGraph.isHead())
					m_headFuncGraphs.push_back(&funcGraph);
			}
		}
	};
};