#pragma once
#include <decompiler/PCode/DecPCode.h>
#include <decompiler/PCode/DecPCodeConstValueCalc.h>
#include <set>

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

		PCodeBlock(int64_t minOffset, int64_t maxOffset);

		void removeRefBlock(PCodeBlock* block);

		void disconnect();

		std::list<PCode::Instruction*>& getInstructions();

		int64_t getMinOffset();

		int64_t getMaxOffset();

		void setMaxOffset(int64_t offset);

		void removeNextBlock(PCodeBlock* nextBlock);

		void setNextNearBlock(PCodeBlock* nextBlock);

		void setNextFarBlock(PCodeBlock* nextBlock);

		PCodeBlock* getNextNearBlock();

		PCodeBlock* getNextFarBlock();

		std::list<PCodeBlock*> getNextBlocks();

		PCode::Instruction* getLastInstruction();

		std::string printDebug(void* addr, const std::string& tabStr, bool extraInfo, bool pcode);
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
		FunctionPCodeGraph(ImagePCodeGraph* imagePCodeGraph);

		ImagePCodeGraph* getImagePCodeGraph();

		void setStartBlock(PCodeBlock* block);

		// head is a function that has not parents (main/all virtual functions)
		bool isHead();

		auto getRefFuncCalls();

		auto getNonVirtFuncCalls();

		auto getVirtFuncCalls();

		void addNonVirtFuncCall(FunctionPCodeGraph* funcGraph);

		void addVirtFuncCall(FunctionPCodeGraph* funcGraph);

		const std::set<PCodeBlock*>& getBlocks();

		void addBlock(PCodeBlock* block);

		PCodeBlock* getStartBlock();

		std::map<PCode::Instruction*, PCode::DataValue>& getConstValues();

		void printDebug(void* addr);
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

		ImagePCodeGraph();

		FunctionPCodeGraph* createFunctionGraph();

		PCodeBlock* createBlock(int64_t minOffset, int64_t maxOffset);

		PCodeBlock* createBlock(int64_t offset);

		const auto& getHeadFuncGraphs();

		std::list<FunctionPCodeGraph>& getFunctionGraphList();

		FunctionPCodeGraph* getEntryFunctionGraph();

		PCodeBlock* getBlockAtOffset(int64_t offset, bool halfInterval = true);

		FunctionPCodeGraph* getFuncGraphAt(int64_t offset, bool halfInterval = true);

		// add all head functions into the list HeadFuncGraphs
		void fillHeadFuncGraphs();
	};
};