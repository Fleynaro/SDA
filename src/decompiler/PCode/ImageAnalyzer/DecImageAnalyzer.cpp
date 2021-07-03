#include "DecImageAnalyzer.h"
#include <decompiler/SDA/Symbolization/DecGraphSdaBuilding.h>
#include <decompiler/Decompiler.h>
#include <managers/TypeManager.h>
#include <managers/SymbolTableManager.h>

using namespace CE::Decompiler;
using namespace CE::Decompiler::PCode;

// pass pcode graph and gather its blocks

CE::Decompiler::ImageAnalyzer::ImageAnalyzer(IImage* image, ImagePCodeGraph* imageGraph, PCode::AbstractDecoder* decoder, AbstractRegisterFactory* registerFactory, PCodeGraphReferenceSearch* graphReferenceSearch)
	: m_image(image), m_imageGraph(imageGraph), m_decoder(decoder), m_registerFactory(registerFactory), m_graphReferenceSearch(graphReferenceSearch)
{}

void CE::Decompiler::ImageAnalyzer::start(int startOffset, bool onceFunc) {
	std::set<int64_t> visitedOffsets;
	std::list<int> nextOffsetsToVisitLater = { startOffset };
	std::list<std::pair<FunctionPCodeGraph*, std::list<int>>> nonVirtFuncOffsetsForGraphs;
	std::map<int, FunctionPCodeGraph*> offsetsToFuncGraphs;
	std::list<PCodeBlock*> blocksToReconnect;

	// generate an image graph
	while (!nextOffsetsToVisitLater.empty()) {
		auto startInstrOffset = (int64_t)nextOffsetsToVisitLater.back() << 8;
		if (visitedOffsets.find(startInstrOffset) != visitedOffsets.end())
			continue;
		visitedOffsets.insert(startInstrOffset);
		nextOffsetsToVisitLater.pop_back();

		auto funcGraph = m_imageGraph->createFunctionGraph();
		try {
			auto block = m_imageGraph->getBlockAtOffset(startInstrOffset);
			// if the function call references to a block of the existing graph
			funcGraph->setStartBlock(block);
			offsetsToFuncGraphs[(int)(startInstrOffset >> 8)] = funcGraph;
			blocksToReconnect.push_back(block);
		}
		catch (ImagePCodeGraph::BlockNotFoundException ex) {
			auto startBlock = m_imageGraph->createBlock(startInstrOffset);
			funcGraph->setStartBlock(startBlock);
			createPCodeBlocksAtOffset(startInstrOffset, funcGraph);
			offsetsToFuncGraphs[(int)(startInstrOffset >> 8)] = funcGraph;

			if (!onceFunc) {
				std::list<int> nonVirtFuncOffsets;
				std::list<int> otherOffsets;
				PrepareFuncGraph(funcGraph);
				m_graphReferenceSearch->findNewFunctionOffsets(funcGraph, nonVirtFuncOffsets, otherOffsets);
				nextOffsetsToVisitLater.insert(nextOffsetsToVisitLater.end(), nonVirtFuncOffsets.begin(), nonVirtFuncOffsets.end());
				nextOffsetsToVisitLater.insert(nextOffsetsToVisitLater.end(), otherOffsets.begin(), otherOffsets.end());
				nonVirtFuncOffsetsForGraphs.push_back(std::make_pair(funcGraph, nonVirtFuncOffsets));
			}
		}
	}

	// associate function graphs with other function graphs through non-virtual calls
	for (auto pair : nonVirtFuncOffsetsForGraphs) {
		auto graph = pair.first;
		auto offsets = pair.second;
		for (auto offset : offsets) {
			auto it = offsetsToFuncGraphs.find(offset);
			if (it != offsetsToFuncGraphs.end()) {
				auto otherGraph = it->second;
				graph->addNonVirtFuncCall(otherGraph);
			}
		}
	}

	// other
	m_imageGraph->fillHeadFuncGraphs();
	reconnectBlocksAndReplaceJmpByCall(blocksToReconnect);
	prepareFuncGraphs();
}

// reconnect all blocks that are referenced by function calls

void CE::Decompiler::ImageAnalyzer::reconnectBlocksAndReplaceJmpByCall(std::list<PCodeBlock*> blocks) const
{
	for (auto block : blocks) {
		for (auto refBlock : block->m_blocksReferencedTo) {
			auto lastInstr = refBlock->getLastInstruction();
			if (PCode::Instruction::IsBranching(lastInstr->m_id)) {
				m_decoder->m_instrPool->modifyInstruction(lastInstr, InstructionPool::MODIFICATOR_JMP_CALL);
				// after modification add new RET instr into ref. block
				auto lastInsertedInstr = &lastInstr->m_origInstruction->m_pcodeInstructions.rbegin()->second;
				refBlock->getInstructions().push_back(lastInsertedInstr);
			}
			refBlock->removeNextBlock(block);
		}
		block->m_blocksReferencedTo.clear();
	}
}

// calculate levels and gather PCode blocks for each function graph

void CE::Decompiler::ImageAnalyzer::prepareFuncGraphs() const
{
	for (auto& funcGraph : m_imageGraph->getFunctionGraphList()) {
		PrepareFuncGraph(&funcGraph);
	}
}

// fill {funcGraph} with PCode blocks

void CE::Decompiler::ImageAnalyzer::createPCodeBlocksAtOffset(int64_t startInstrOffset, FunctionPCodeGraph* funcGraph) const
{
	std::set<int64_t> visitedOffsets;
	std::list<int64_t> nextOffsetsToVisitLater;

	auto offset = startInstrOffset;
	while (true) {
		auto byteOffset = (int)(offset >> 8);
		auto instrOrder = offset & 0xFF;
		PCode::Instruction* instr = nullptr;
		PCodeBlock* curBlock = nullptr;

		if (offset != -1 && visitedOffsets.find(offset) == visitedOffsets.end()) {
			try {
				// any offset have to be assoicated with some existing block
				curBlock = m_imageGraph->getBlockAtOffset(offset, false);

				// try to get an instruction by the offset
				try {
					instr = m_decoder->m_instrPool->getInstructionAt(offset);
				}
				catch (InstructionPool::InstructionNotFoundException ex) {
					// if no instruction at the offset then decode the location
					if (byteOffset < m_image->getSize()) {
						m_decoder->decode(m_image->getData() + m_image->toImageOffset(byteOffset), byteOffset, m_image->getSize());
					}

					try {
						instr = m_decoder->m_instrPool->getInstructionAt(offset);
					}
					catch (InstructionPool::InstructionNotFoundException ex) {
					}
				}
				visitedOffsets.insert(offset);
			}
			catch (ImagePCodeGraph::BlockNotFoundException ex) {}
		}

		if (instr == nullptr) {
			// select the next new block to visit
			offset = -1;
			while (!nextOffsetsToVisitLater.empty()) {
				offset = nextOffsetsToVisitLater.back();
				nextOffsetsToVisitLater.pop_back();
				if (visitedOffsets.find(offset) == visitedOffsets.end())
					break;
				offset = -1;
			}

			// visit a new offset
			if (offset != -1)
				continue;

			// if no new offsets then exit
			break;
		}

		curBlock->getInstructions().push_back(instr);
		// calculate offset of the next instruction
		auto nextInstrOffset = instr->getOffset() + 1;
		bool needChangeNextInstrOffset = false;
		try {
			auto instr = m_decoder->m_instrPool->getInstructionAt(nextInstrOffset);
			if (byteOffset != instr->m_origInstruction->m_offset)
				needChangeNextInstrOffset = true;
		}
		catch (InstructionPool::InstructionNotFoundException ex) {
			needChangeNextInstrOffset = true;
		}
		if (needChangeNextInstrOffset)
			nextInstrOffset = instr->getFirstInstrOffsetInNextOrigInstr();
		// extend size of the current block
		curBlock->setMaxOffset(nextInstrOffset);

		// create a new block
		if (PCode::Instruction::IsBranching(instr->m_id)) {
			PCode::VirtualMachineContext vmCtx;
			PCode::ConstValueCalculating constValueCalculating(curBlock->getInstructions(), &vmCtx, m_registerFactory);
			constValueCalculating.start(funcGraph->getConstValues());

			int64_t targetOffset = -1;
			if (auto varnodeConst = dynamic_cast<PCode::ConstantVarnode*>(instr->m_input0)) {
				// if this input contains hardcoded constant
				targetOffset = varnodeConst->m_value;
			}
			else {
				// if this input could be constantly calculated by pcode virtual machine
				auto it = funcGraph->getConstValues().find(instr);
				if (it != funcGraph->getConstValues().end())
					targetOffset = it->second << 8;
			}

			if (targetOffset == -1 || m_image->defineSegment((int)(targetOffset >> 8)) != IImage::CODE_SEGMENT) {
				offset = -1;
				m_decoder->getWarningContainer()->addWarning("rva " + std::to_string(targetOffset >> 8) + " is not correct in the jump instruction " + instr->m_origInstruction->m_originalView + " (at 0x" + Helper::String::NumberToHex(instr->m_origInstruction->m_offset) + ")");
				continue;
			}

			// far block
			PCodeBlock* nextFarBlock = nullptr;
			try {
				auto alreadyExistingBlock = m_imageGraph->getBlockAtOffset(targetOffset);
				// split the already existing block into 2 non-empty blocks 
				if (targetOffset > alreadyExistingBlock->getMinOffset() && targetOffset < alreadyExistingBlock->getMaxOffset() - 1) {
					auto block1 = alreadyExistingBlock;
					auto block2 = m_imageGraph->createBlock(targetOffset);

					std::list<PCode::Instruction*> instrOfBlock1;
					std::list<PCode::Instruction*> instrOfBlock2;
					for (auto instr : alreadyExistingBlock->getInstructions()) {
						if (instr->getOffset() < targetOffset)
							instrOfBlock1.push_back(instr);
						else instrOfBlock2.push_back(instr);
					}
					block1->getInstructions() = instrOfBlock1;
					block2->getInstructions() = instrOfBlock2;

					block2->setMaxOffset(alreadyExistingBlock->getMaxOffset());
					block1->setMaxOffset(targetOffset);

					if (block1->getNextNearBlock())
						block2->setNextNearBlock(block1->getNextNearBlock());
					if (block1->getNextFarBlock())
						block2->setNextFarBlock(block1->getNextFarBlock());
					block1->disconnect();
					block1->setNextNearBlock(block2);

					if (curBlock == alreadyExistingBlock)
						alreadyExistingBlock = curBlock = block2;
				}
				curBlock->setNextFarBlock(alreadyExistingBlock);
			}
			catch (ImagePCodeGraph::BlockNotFoundException ex) {
				nextFarBlock = m_imageGraph->createBlock(targetOffset);
				curBlock->setNextFarBlock(nextFarBlock);
			}

			// near block
			PCodeBlock* nextNearBlock = nullptr;
			if (instr->m_id == PCode::InstructionId::CBRANCH) {
				try {
					m_imageGraph->getBlockAtOffset(nextInstrOffset);
				}
				catch (ImagePCodeGraph::BlockNotFoundException ex) {
					nextNearBlock = m_imageGraph->createBlock(nextInstrOffset);
					curBlock->setNextNearBlock(nextNearBlock);
				}
			}

			// calculate the next offset (selecting the next following block if possible)
			if (nextNearBlock) {
				offset = nextNearBlock->getMinOffset();
				if (nextFarBlock) {
					nextOffsetsToVisitLater.push_back(nextFarBlock->getMinOffset());
				}
			}
			else if (nextFarBlock) {
				offset = nextFarBlock->getMinOffset();
			}
			else {
				offset = -1;
			}
		}
		else {
			// calculate the next offset
			if (instr->m_id != PCode::InstructionId::RETURN) {
				try {
					auto nextBlock = m_imageGraph->getBlockAtOffset(nextInstrOffset, false);
					if (curBlock != nextBlock)
						curBlock->setNextNearBlock(nextBlock);
				}
				catch (ImagePCodeGraph::BlockNotFoundException ex) {}
				offset = nextInstrOffset;
			}
			else {
				offset = -1;
			}
		}
	}
}

// prepare a function graph

void CE::Decompiler::ImageAnalyzer::PrepareFuncGraph(FunctionPCodeGraph* funcGraph) {
	std::list<PCodeBlock*> path;
	CalculateLevelsForPCodeBlocks(funcGraph->getStartBlock(), path);

	std::set<PCodeBlock*> blocks;
	GatherPCodeBlocks(funcGraph->getStartBlock(), blocks);
	for (auto block : blocks) {
		funcGraph->addBlock(block);
	}
}

// pass pcode graph and calculate max distance from root to each node (pcode block)

void CE::Decompiler::ImageAnalyzer::CalculateLevelsForPCodeBlocks(PCodeBlock* block, std::list<PCodeBlock*>& path) {
	if (block == nullptr)
		return;

	//check if there's a loop
	for (auto it = path.rbegin(); it != path.rend(); it++) {
		if (block == *it) {
			return;
		}
	}

	path.push_back(block);
	block->m_level = std::max(block->m_level, (int)path.size());
	CalculateLevelsForPCodeBlocks(block->getNextNearBlock(), path);
	CalculateLevelsForPCodeBlocks(block->getNextFarBlock(), path);
	path.pop_back();
}

void CE::Decompiler::ImageAnalyzer::GatherPCodeBlocks(PCodeBlock* block, std::set<PCodeBlock*>& gatheredBlocks) {
	if (block == nullptr || gatheredBlocks.find(block) != gatheredBlocks.end())
		return;
	gatheredBlocks.insert(block);
	GatherPCodeBlocks(block->getNextNearBlock(), gatheredBlocks);
	GatherPCodeBlocks(block->getNextFarBlock(), gatheredBlocks);
}

CE::Decompiler::PCodeGraphReferenceSearch::PCodeGraphReferenceSearch(CE::Project* project, AbstractRegisterFactory* registerFactory, IImage* image)
	: m_project(project), m_registerFactory(registerFactory), m_image(image)
{
	auto factory = m_project->getSymTableManager()->getFactory(false);
	m_symbolCtx.m_signature = nullptr;
	m_symbolCtx.m_globalSymbolTable = factory.createSymbolTable(CE::Symbol::SymbolTable::GLOBAL_SPACE);
	m_symbolCtx.m_stackSymbolTable = factory.createSymbolTable(CE::Symbol::SymbolTable::STACK_SPACE);
	m_symbolCtx.m_funcBodySymbolTable = factory.createSymbolTable(CE::Symbol::SymbolTable::GLOBAL_SPACE);
}

CE::Decompiler::PCodeGraphReferenceSearch::~PCodeGraphReferenceSearch() {
	delete m_symbolCtx.m_globalSymbolTable;
	delete m_symbolCtx.m_stackSymbolTable;
	delete m_symbolCtx.m_funcBodySymbolTable;
}

void CE::Decompiler::PCodeGraphReferenceSearch::findNewFunctionOffsets(FunctionPCodeGraph* funcGraph, std::list<int>& nonVirtFuncOffsets, std::list<int>& otherOffsets) {
	auto funcCallInfoCallback = [&](PCode::Instruction* instr, int offset) { return FunctionCallInfo({}); };
	auto decompiler = CE::Decompiler::Decompiler(funcGraph, funcCallInfoCallback, ReturnInfo(), m_registerFactory);
	decompiler.start();

	auto decCodeGraph = decompiler.getDecGraph();
	auto sdaCodeGraph = new SdaCodeGraph(decCodeGraph);
	Symbolization::SdaBuilding sdaBuilding(sdaCodeGraph, &m_symbolCtx, m_project);
	sdaBuilding.start();

	for (auto symbol : sdaBuilding.getNewAutoSymbols()) {
		if (auto memSymbol = dynamic_cast<CE::Symbol::IMemorySymbol*>(symbol)) {
			auto storage = memSymbol->getStorage();
			auto offset = (int)storage.getOffset();
			if (storage.getType() == Storage::STORAGE_GLOBAL) {
				auto segmentType = m_image->defineSegment(offset);
				if (segmentType == IImage::CODE_SEGMENT) {
					nonVirtFuncOffsets.push_back(offset);
				}
				else if (segmentType == IImage::DATA_SEGMENT) {
					auto imageOffset = m_image->toImageOffset(offset);
					VTable* pVtable = nullptr;
					checkOnVTable(imageOffset, pVtable);
					if (pVtable) {
						for (auto funcOffset : pVtable->m_funcOffsets)
							otherOffsets.push_back(funcOffset);
					}
				}
			}
		}
		delete symbol;
	}

	delete decCodeGraph;
	delete sdaCodeGraph;
}

// analyze memory area to define if it is a vtable

void CE::Decompiler::PCodeGraphReferenceSearch::checkOnVTable(int startOffset, VTable* pVtable) {
	std::list<int64_t> funcOffsets;
	auto data = m_image->getData();
	for (int offset = startOffset; offset < m_image->getSize(); offset += sizeof(uint64_t)) {
		auto funcAddr = (uint64_t&)data[offset];
		// all vtables ends with zero address
		if (funcAddr == 0x0)
			break;
		auto funcOffset = m_image->addrToImageOffset(funcAddr);
		if (m_image->defineSegment(funcOffset) != IImage::CODE_SEGMENT)
			break;
		funcOffsets.push_back(funcOffset);
	}

	if (funcOffsets.empty())
		return;

	// we found a vtable
	VTable vtable;
	vtable.m_offset = startOffset;
	vtable.m_funcOffsets = funcOffsets;
	m_vtables.push_back(vtable);
	pVtable = &*m_vtables.rbegin();
}
