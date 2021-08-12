#include "DecImageAnalyzer.h"
#include <decompiler/SDA/Symbolization/DecGraphSdaBuilding.h>
#include <decompiler/Decompiler.h>
#include <managers/TypeManager.h>
#include <managers/SymbolTableManager.h>

using namespace CE::Decompiler;
using namespace PCode;

// pass pcode graph and gather its blocks

ImageAnalyzer::ImageAnalyzer(AbstractImage* image, ImagePCodeGraph* imageGraph, AbstractDecoder* decoder, AbstractRegisterFactory* registerFactory, PCodeGraphReferenceSearch* graphReferenceSearch)
	: m_image(image), m_imageGraph(imageGraph), m_decoder(decoder), m_registerFactory(registerFactory), m_graphReferenceSearch(graphReferenceSearch)
{}

void ImageAnalyzer::start(Offset startOffset, bool onceFunc) const {
	std::set<Offset> visitedOffsets;
	std::list<Offset> nextOffsetsToVisitLater = { startOffset };
	std::list<std::pair<FunctionPCodeGraph*, std::list<Offset>>> nonVirtFuncOffsetsForGraphs;
	std::map<Offset, FunctionPCodeGraph*> offsetsToFuncGraphs;
	std::list<PCodeBlock*> blocksToReconnect;

	// generate an image graph
	while (!nextOffsetsToVisitLater.empty()) {
		auto startInstrOffset = ComplexOffset(nextOffsetsToVisitLater.back(), 0);
		if (visitedOffsets.find(startInstrOffset) != visitedOffsets.end())
			continue;
		visitedOffsets.insert(startInstrOffset);
		nextOffsetsToVisitLater.pop_back();

		auto funcGraph = m_imageGraph->createFunctionGraph();
		if (auto block = m_imageGraph->getBlockAtOffset(startInstrOffset)) {
			// if the function call references to a block of the existing graph
			funcGraph->setStartBlock(block);
			offsetsToFuncGraphs[startInstrOffset.getByteOffset()] = funcGraph;
			blocksToReconnect.push_back(block);
		}
		else {
			const auto startBlock = m_imageGraph->createBlock(startInstrOffset);
			funcGraph->setStartBlock(startBlock);
			createPCodeBlocksAtOffset(startInstrOffset, funcGraph);
			offsetsToFuncGraphs[startInstrOffset.getByteOffset()] = funcGraph;

			if (!onceFunc) {
				std::list<Offset> nonVirtFuncOffsets;
				std::list<Offset> otherOffsets;
				PrepareFuncGraph(funcGraph);
				m_graphReferenceSearch->findNewFunctionOffsets(funcGraph, nonVirtFuncOffsets, otherOffsets);
				nextOffsetsToVisitLater.insert(nextOffsetsToVisitLater.end(), nonVirtFuncOffsets.begin(), nonVirtFuncOffsets.end());
				nextOffsetsToVisitLater.insert(nextOffsetsToVisitLater.end(), otherOffsets.begin(), otherOffsets.end());
				nonVirtFuncOffsetsForGraphs.emplace_back(funcGraph, nonVirtFuncOffsets);
			}
		}
	}

	// associate function graphs with other function graphs through non-virtual calls
	for (const auto& [graph, offsets]: nonVirtFuncOffsetsForGraphs) {
		for (auto offset : offsets) {
			auto it = offsetsToFuncGraphs.find(offset);
			if (it != offsetsToFuncGraphs.end()) {
				const auto otherGraph = it->second;
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

void ImageAnalyzer::reconnectBlocksAndReplaceJmpByCall(std::list<PCodeBlock*> blocks) const
{
	for (auto block : blocks) {
		for (auto refBlock : block->m_blocksReferencedTo) {
			auto lastInstr = refBlock->getLastInstruction();
			if (Instruction::IsBranching(lastInstr->m_id)) {
				m_decoder->m_instrPool->modifyInstruction(lastInstr, InstructionPool::MODIFICATOR_JMP_CALL);
				// after modification add new RET instr into ref. block
				auto lastInsertedInstr = &lastInstr->m_origInstruction->m_pcodeInstructions.rbegin()->second;
				refBlock->m_instructions.push_back(lastInsertedInstr);
			}
			refBlock->removeNextBlock(block);
		}
		block->m_blocksReferencedTo.clear();
	}
}

// calculate levels and gather PCode blocks for each function graph

void ImageAnalyzer::prepareFuncGraphs() const
{
	for (auto& funcGraph : m_imageGraph->getFunctionGraphList()) {
		PrepareFuncGraph(&funcGraph);
	}
}

// fill {funcGraph} with PCode blocks

void ImageAnalyzer::createPCodeBlocksAtOffset(ComplexOffset startInstrOffset, FunctionPCodeGraph* funcGraph) const
{
	std::set<ComplexOffset> visitedOffsets;
	std::list<ComplexOffset> nextOffsetsToVisitLater;

	auto offset = startInstrOffset;
	while (true) {
		const auto byteOffset = offset.getByteOffset();
		Instruction* instr = nullptr;
		PCodeBlock* curBlock = nullptr;

		if (offset != InvalidOffset && visitedOffsets.find(offset) == visitedOffsets.end()) {
			// any offset have to be assoicated with some existing block
			if ((curBlock = m_imageGraph->getBlockAtOffset(offset, false))) {

				// try to get an instruction by the offset
				instr = m_decoder->m_instrPool->getPCodeInstructionAt(offset);
				if (!instr) {
					// if no instruction at the offset then decode the location
					if (byteOffset < m_image->getSize()) {
						std::vector<uint8_t> buffer(100);
						m_image->getReader()->read(byteOffset, buffer);
						m_decoder->decode(byteOffset, buffer);
					}
					instr = m_decoder->m_instrPool->getPCodeInstructionAt(offset);
				}
				visitedOffsets.insert(offset);
			}
		}

		if (instr == nullptr) {
			// select the next new block to visit
			offset = InvalidOffset;
			while (!nextOffsetsToVisitLater.empty()) {
				offset = nextOffsetsToVisitLater.back();
				nextOffsetsToVisitLater.pop_back();
				if (visitedOffsets.find(offset) == visitedOffsets.end())
					break;
				offset = InvalidOffset;
			}

			// visit a new offset
			if (offset != InvalidOffset)
				continue;

			// if no new offsets then exit
			break;
		}

		curBlock->m_instructions.push_back(instr);
		// calculate offset of the next instruction
		auto nextInstrOffset = ComplexOffset(instr->getOffset() + 1);
		bool needChangeNextInstrOffset = false;
		if (const auto instr = m_decoder->m_instrPool->getPCodeInstructionAt(nextInstrOffset)) {
			if (byteOffset != instr->m_origInstruction->m_offset)
				needChangeNextInstrOffset = true;
		}
		else {
			needChangeNextInstrOffset = true;
		}
		if (needChangeNextInstrOffset)
			nextInstrOffset = instr->getFirstInstrOffsetInNextOrigInstr();
		// extend size of the current block
		curBlock->setMaxOffset(nextInstrOffset);

		// fill const values
		if (Instruction::IsAnyJmup(instr->m_id)) {
			ConstValueCalculating constValueCalculating(curBlock->getInstructions(), m_registerFactory);
			constValueCalculating.start(funcGraph->getConstValues());
		}

		// create a new block
		if (Instruction::IsBranching(instr->m_id)) {
			ComplexOffset targetOffset;
			if (const auto varnodeConst = dynamic_cast<ConstantVarnode*>(instr->m_input0)) {
				// if this input contains hardcoded constant
				targetOffset = varnodeConst->m_value;
			}
			else {
				// if this input could be constantly calculated by pcode virtual machine
				auto it = funcGraph->getConstValues().find(instr);
				if (it != funcGraph->getConstValues().end())
					targetOffset = ComplexOffset(it->second, 0);
			}

			if (targetOffset == InvalidOffset || m_image->getSectionByOffset(targetOffset.getByteOffset())->m_type != ImageSection::CODE_SEGMENT) {
				offset = InvalidOffset;
				m_decoder->getWarningContainer()->addWarning(
					"rva " + std::to_string(targetOffset >> 8) + " is not correct in the jump instruction " + instr->m_origInstruction->m_originalView + " (at 0x" + Helper::String::NumberToHex(instr->m_origInstruction->m_offset) + ")");
				continue;
			}

			// far block
			PCodeBlock* nextFarBlock = nullptr;
			if (auto alreadyExistingBlock = m_imageGraph->getBlockAtOffset(targetOffset)) {
				// split the already existing block into 2 non-empty blocks 
				if (targetOffset > alreadyExistingBlock->getMinOffset() && targetOffset < alreadyExistingBlock->getMaxOffset() - 1) {
					auto block1 = alreadyExistingBlock;
					auto block2 = m_imageGraph->createBlock(targetOffset);

					std::list<Instruction*> instrOfBlock1;
					std::list<Instruction*> instrOfBlock2;
					for (const auto instr : alreadyExistingBlock->getInstructions()) {
						if (instr->getOffset() < targetOffset)
							instrOfBlock1.push_back(instr);
						else instrOfBlock2.push_back(instr);
					}
					block1->m_instructions = instrOfBlock1;
					block2->m_instructions = instrOfBlock2;

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
			else {
				nextFarBlock = m_imageGraph->createBlock(targetOffset);
				curBlock->setNextFarBlock(nextFarBlock);
			}

			// near block
			PCodeBlock* nextNearBlock = nullptr;
			if (instr->m_id == InstructionId::CBRANCH) {
				if (m_imageGraph->getBlockAtOffset(nextInstrOffset) == nullptr) {
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
				offset = InvalidOffset;
			}
		}
		else {
			// calculate the next offset
			if (instr->m_id != InstructionId::RETURN) {
				if (const auto nextBlock = m_imageGraph->getBlockAtOffset(nextInstrOffset, false)) {
					if (curBlock != nextBlock)
						curBlock->setNextNearBlock(nextBlock);
				}
				offset = nextInstrOffset;
			}
			else {
				offset = InvalidOffset;
			}
		}
	}
}

// prepare a function graph

void ImageAnalyzer::PrepareFuncGraph(FunctionPCodeGraph* funcGraph) {
	std::list<PCodeBlock*> path;
	CalculateLevelsForPCodeBlocks(funcGraph->getStartBlock(), path);

	std::set<PCodeBlock*> blocks;
	GatherPCodeBlocks(funcGraph->getStartBlock(), blocks);
	for (auto block : blocks) {
		funcGraph->addBlock(block);
	}
}

// pass pcode graph and calculate max distance from root to each node (pcode block)

void ImageAnalyzer::CalculateLevelsForPCodeBlocks(PCodeBlock* block, std::list<PCodeBlock*>& path) {
	if (block == nullptr)
		return;

	//check if there's a loop
	for (auto it = path.rbegin(); it != path.rend(); it++) {
		if (block == *it) {
			return;
		}
	}

	path.push_back(block);
	block->m_level = std::max(block->m_level, static_cast<int>(path.size()));
	CalculateLevelsForPCodeBlocks(block->getNextNearBlock(), path);
	CalculateLevelsForPCodeBlocks(block->getNextFarBlock(), path);
	path.pop_back();
}

void ImageAnalyzer::GatherPCodeBlocks(PCodeBlock* block, std::set<PCodeBlock*>& gatheredBlocks) {
	if (block == nullptr || gatheredBlocks.find(block) != gatheredBlocks.end())
		return;
	gatheredBlocks.insert(block);
	GatherPCodeBlocks(block->getNextNearBlock(), gatheredBlocks);
	GatherPCodeBlocks(block->getNextFarBlock(), gatheredBlocks);
}

PCodeGraphReferenceSearch::PCodeGraphReferenceSearch(Project* project, AbstractRegisterFactory* registerFactory, AbstractImage* image)
	: m_project(project), m_registerFactory(registerFactory), m_image(image)
{
	const auto factory = m_project->getSymTableManager()->getFactory(false);
	m_symbolCtx.m_signature = nullptr;
	m_symbolCtx.m_globalSymbolTable = factory.createGlobalSymbolTable();
	m_symbolCtx.m_stackSymbolTable = factory.createStackSymbolTable();
	m_symbolCtx.m_funcBodySymbolTable = factory.createGlobalSymbolTable();
}

PCodeGraphReferenceSearch::~PCodeGraphReferenceSearch() {
	delete m_symbolCtx.m_globalSymbolTable;
	delete m_symbolCtx.m_stackSymbolTable;
	delete m_symbolCtx.m_funcBodySymbolTable;
}

void PCodeGraphReferenceSearch::findNewFunctionOffsets(FunctionPCodeGraph* funcGraph, std::list<uint64_t>& nonVirtFuncOffsets, std::list<uint64_t>& otherOffsets) {
	auto funcCallInfoCallback = [&](Instruction* instr, int offset) { return FunctionCallInfo({}); };
	auto decompiler = Decompiler(funcGraph, funcCallInfoCallback, ReturnInfo(), m_registerFactory);
	decompiler.start();

	const auto decCodeGraph = decompiler.getDecGraph();
	const auto sdaCodeGraph = new SdaCodeGraph(decCodeGraph);
	Symbolization::SdaBuilding sdaBuilding(sdaCodeGraph, &m_symbolCtx, m_project);
	sdaBuilding.start();

	for (auto symbol : sdaBuilding.getNewAutoSymbols()) {
		if (const auto memSymbol = dynamic_cast<CE::Symbol::AbstractMemorySymbol*>(symbol)) {
			const auto offset = static_cast<Offset>(memSymbol->getOffset());
			if (memSymbol->getType() == CE::Symbol::GLOBAL_VAR) {
				const auto segmentType = m_image->getSectionByOffset(offset)->m_type;
				if (segmentType == ImageSection::CODE_SEGMENT) {
					nonVirtFuncOffsets.push_back(offset);
				}
				else if (segmentType == ImageSection::DATA_SEGMENT) {
					const auto imageOffset = m_image->toImageOffset(offset);
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

void PCodeGraphReferenceSearch::checkOnVTable(uint64_t startOffset, VTable* pVtable) {
	std::list<uint64_t> funcOffsets;
	for (auto offset = startOffset; offset < m_image->getSize(); offset += sizeof(uint64_t)) {
		std::vector<uint8_t> buffer(sizeof uint64_t);
		m_image->getReader()->read(offset, buffer);
		const auto funcAddr = *reinterpret_cast<uint64_t*>(buffer.data());
		// all vtables ends with zero address
		if (funcAddr == 0x0)
			break;
		const auto funcRva = m_image->addressToOffset(funcAddr);
		if (m_image->getSectionByOffset(funcRva)->m_type != ImageSection::CODE_SEGMENT)
			break;
		funcOffsets.push_back(funcRva);
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
