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
		IImage* m_image;
		SymbolContext m_symbolCtx;
	public:
		struct VTable {
			int64_t m_offset;
			std::list<int64_t> m_funcOffsets;
		};
		std::list<VTable> m_vtables;

		PCodeGraphReferenceSearch(CE::Project* project, AbstractRegisterFactory* registerFactory, IImage* image);

		~PCodeGraphReferenceSearch();

		void findNewFunctionOffsets(FunctionPCodeGraph* funcGraph, std::list<int>& nonVirtFuncOffsets, std::list<int>& otherOffsets);

	private:
		// analyze memory area to define if it is a vtable
		void checkOnVTable(int startOffset, VTable* pVtable);
	};

	// Analysis of an image of some program (.exe or .dll)
	class ImageAnalyzer
	{
		IImage* m_image;
		ImagePCodeGraph* m_imageGraph = nullptr;
		PCodeGraphReferenceSearch* m_graphReferenceSearch;

		AbstractRegisterFactory* m_registerFactory;
		PCode::AbstractDecoder* m_decoder;
	public:
		ImageAnalyzer(IImage* image, ImagePCodeGraph* imageGraph, PCode::AbstractDecoder* decoder, AbstractRegisterFactory* registerFactory, PCodeGraphReferenceSearch* graphReferenceSearch = nullptr);

		void start(int startOffset, bool onceFunc = false);

	private:
		// reconnect all blocks that are referenced by function calls
		void reconnectBlocksAndReplaceJmpByCall(std::list<PCodeBlock*> blocks) const;

		// calculate levels and gather PCode blocks for each function graph
		void prepareFuncGraphs() const;

		// fill {funcGraph} with PCode blocks
		void createPCodeBlocksAtOffset(int64_t startInstrOffset, FunctionPCodeGraph* funcGraph) const;

		// prepare a function graph
		static void PrepareFuncGraph(FunctionPCodeGraph* funcGraph);

		// pass pcode graph and calculate max distance from root to each node (pcode block)
		static void CalculateLevelsForPCodeBlocks(PCodeBlock* block, std::list<PCodeBlock*>& path);

		// pass pcode graph and gather its blocks
		static void GatherPCodeBlocks(PCodeBlock* block, std::set<PCodeBlock*>& gatheredBlocks);
	};
};