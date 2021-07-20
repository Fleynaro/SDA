#pragma once
#include <decompiler/PCode/DecPCode.h>
#include <decompiler/PCode/DecPCodeConstValueCalc.h>
#include <set>

namespace DB
{
	class ImageMapper;
};

namespace CE::Decompiler
{
	class ImagePCodeGraph;
	class FunctionPCodeGraph;

	// pcode graph for a non-branched block
	class PCodeBlock
	{
		friend class ImageAnalyzer;
		friend class DB::ImageMapper;
		ComplexOffset m_minOffset;
		ComplexOffset m_maxOffset;
		std::list<Instruction*> m_instructions; // content of the block
		PCodeBlock* m_nextNearBlock = nullptr;
		PCodeBlock* m_nextFarBlock = nullptr;
	public:
		std::string m_name;
		int m_level = 0;
		std::list<PCodeBlock*> m_blocksReferencedTo;
		FunctionPCodeGraph* m_funcPCodeGraph = nullptr;

		PCodeBlock() = default;

		PCodeBlock(ComplexOffset minOffset, ComplexOffset maxOffset);

		std::string getName() const;

		void removeRefBlock(PCodeBlock* block);

		void disconnect();

		std::list<PCodeBlock*> getRefHighBlocks() const;

		const std::list<Instruction*>& getInstructions() const;

		ComplexOffset getMinOffset() const;

		ComplexOffset getMaxOffset() const;

		void setMaxOffset(ComplexOffset offset);

		void removeNextBlock(PCodeBlock* nextBlock);

		void setNextNearBlock(PCodeBlock* nextBlock);

		void setNextFarBlock(PCodeBlock* nextBlock);

		PCodeBlock* getNextNearBlock() const;

		PCodeBlock* getNextFarBlock() const;

		std::list<PCodeBlock*> getNextBlocks() const;

		Instruction* getLastInstruction();
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
		std::map<Instruction*, DataValue> m_constValues;
	public:
		FunctionPCodeGraph(ImagePCodeGraph* imagePCodeGraph);

		ImagePCodeGraph* getImagePCodeGraph() const;

		void setStartBlock(PCodeBlock* block);

		// head is a function that has not parents (main/all virtual functions)
		bool isHead() const;

		const std::set<FunctionPCodeGraph*>& getRefFuncCalls() const;

		const std::set<FunctionPCodeGraph*>& getNonVirtFuncCalls() const;

		const std::set<FunctionPCodeGraph*>& getVirtFuncCalls() const;

		void addNonVirtFuncCall(FunctionPCodeGraph* funcGraph);

		void addVirtFuncCall(FunctionPCodeGraph* funcGraph);

		const std::set<PCodeBlock*>& getBlocks() const;

		void addBlock(PCodeBlock* block);

		PCodeBlock* getStartBlock() const;

		std::map<Instruction*, DataValue>& getConstValues();
	};

	// pcode graph (consisted of NON-connected function graphs in final state) for a whole program
	class ImagePCodeGraph
	{
		std::list<FunctionPCodeGraph> m_funcGraphList;
		std::list<FunctionPCodeGraph*> m_headFuncGraphs;
		std::map<ComplexOffset, PCodeBlock> m_blocks;
	public:
		ImagePCodeGraph();

		FunctionPCodeGraph* createFunctionGraph();

		PCodeBlock* createBlock(ComplexOffset minOffset, ComplexOffset maxOffset);

		PCodeBlock* createBlock(ComplexOffset offset);

		const std::list<FunctionPCodeGraph*>& getHeadFuncGraphs() const;

		std::list<FunctionPCodeGraph>& getFunctionGraphList();

		FunctionPCodeGraph* getEntryFunctionGraph();

		PCodeBlock* getBlockAtOffset(ComplexOffset offset, bool halfInterval = true);

		FunctionPCodeGraph* getFuncGraphAt(ComplexOffset offset, bool halfInterval = true);

		// add all head functions into the list HeadFuncGraphs
		void fillHeadFuncGraphs();
	};
};