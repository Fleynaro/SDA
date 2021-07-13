#pragma once
#include <Project.h>
#include <Function.h>
#include <images/PEImage.h>
#include <decompiler/Graph/DecCodeGraph.h>
#include <decompiler/PCode/Decoders/DecPCodeDecoderX86.h>

namespace CE::Decompiler
{
	class PCodeGraphReferenceSearch
	{
		Project* m_project;
		AbstractRegisterFactory* m_registerFactory;
		AbstractImage* m_image;
		SymbolContext m_symbolCtx;
	public:
		struct VTable {
			uint64_t m_offset;
			std::list<uint64_t> m_funcOffsets;
		};
		std::list<VTable> m_vtables;

		PCodeGraphReferenceSearch(CE::Project* project, AbstractRegisterFactory* registerFactory, AbstractImage* image);

		~PCodeGraphReferenceSearch();

		void findNewFunctionOffsets(FunctionPCodeGraph* funcGraph, std::list<uint64_t>& nonVirtFuncOffsets, std::list<uint64_t>& otherOffsets);

	private:
		// analyze memory area to define if it is a vtable
		void checkOnVTable(uint64_t startOffset, VTable* pVtable);
	};

	// Analysis of an image of some program (.exe or .dll)
	class ImageAnalyzer
	{
		AbstractImage* m_image;
		ImagePCodeGraph* m_imageGraph = nullptr;
		PCodeGraphReferenceSearch* m_graphReferenceSearch;

		AbstractRegisterFactory* m_registerFactory;
		PCode::AbstractDecoder* m_decoder;
	public:
		ImageAnalyzer(AbstractImage* image, ImagePCodeGraph* imageGraph, PCode::AbstractDecoder* decoder, AbstractRegisterFactory* registerFactory, PCodeGraphReferenceSearch* graphReferenceSearch = nullptr);

		void start(Offset startOffset, bool onceFunc = false) const;

	private:
		// reconnect all blocks that are referenced by function calls
		void reconnectBlocksAndReplaceJmpByCall(std::list<PCodeBlock*> blocks) const;

		// calculate levels and gather PCode blocks for each function graph
		void prepareFuncGraphs() const;

		// fill {funcGraph} with PCode blocks
		void createPCodeBlocksAtOffset(ComplexOffset startInstrOffset, FunctionPCodeGraph* funcGraph) const;

		// prepare a function graph
		static void PrepareFuncGraph(FunctionPCodeGraph* funcGraph);

		// pass pcode graph and calculate max distance from root to each node (pcode block)
		static void CalculateLevelsForPCodeBlocks(PCodeBlock* block, std::list<PCodeBlock*>& path);

		// pass pcode graph and gather its blocks
		static void GatherPCodeBlocks(PCodeBlock* block, std::set<PCodeBlock*>& gatheredBlocks);
	};
};