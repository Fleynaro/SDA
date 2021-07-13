#include "DecPCodeGraph.h"
#include <utilities/Helper.h>

using namespace CE::Decompiler;

// add all head functions into the list HeadFuncGraphs

CE::Decompiler::ImagePCodeGraph::ImagePCodeGraph()
{}

FunctionPCodeGraph* CE::Decompiler::ImagePCodeGraph::createFunctionGraph() {
	m_funcGraphList.push_back(FunctionPCodeGraph(this));
	return &*m_funcGraphList.rbegin();
}

PCodeBlock* CE::Decompiler::ImagePCodeGraph::createBlock(ComplexOffset minOffset, ComplexOffset maxOffset) {
	m_blocks.insert(std::make_pair(minOffset, PCodeBlock(minOffset, maxOffset)));
	const auto newBlock = &m_blocks[minOffset];
	return newBlock;
}

PCodeBlock* CE::Decompiler::ImagePCodeGraph::createBlock(ComplexOffset offset) {
	return createBlock(offset, offset);
}

const auto& CE::Decompiler::ImagePCodeGraph::getHeadFuncGraphs() const
{
	return m_headFuncGraphs;
}

std::list<FunctionPCodeGraph>& CE::Decompiler::ImagePCodeGraph::getFunctionGraphList() {
	return m_funcGraphList;
}

FunctionPCodeGraph* CE::Decompiler::ImagePCodeGraph::getEntryFunctionGraph() {
	return &*m_funcGraphList.begin();
}

PCodeBlock* CE::Decompiler::ImagePCodeGraph::getBlockAtOffset(ComplexOffset offset, bool halfInterval) {
	if (!m_blocks.empty()) {
		auto it = std::prev(m_blocks.upper_bound(offset));
		if (it != m_blocks.end()) {
			const bool boundUp = halfInterval ? (offset < it->second.getMaxOffset()) : (offset <= it->second.getMaxOffset());
			if (boundUp && offset >= it->second.getMinOffset()) {
				return &it->second;
			}
		}
	}
	return nullptr;
}

FunctionPCodeGraph* CE::Decompiler::ImagePCodeGraph::getFuncGraphAt(ComplexOffset offset, bool halfInterval) {
	const auto block = getBlockAtOffset(offset, halfInterval);
	return block->m_funcPCodeGraph;
}

void CE::Decompiler::ImagePCodeGraph::fillHeadFuncGraphs() {
	for (auto& funcGraph : getFunctionGraphList()) {
		if (funcGraph.isHead())
			m_headFuncGraphs.push_back(&funcGraph);
	}
}

CE::Decompiler::PCodeBlock::PCodeBlock(ComplexOffset minOffset, ComplexOffset maxOffset)
	: m_minOffset(minOffset), m_maxOffset(maxOffset), ID(static_cast<int>(minOffset >> 8))
{}

void CE::Decompiler::PCodeBlock::removeRefBlock(PCodeBlock * block) {
	m_blocksReferencedTo.remove(block);
}

void CE::Decompiler::PCodeBlock::disconnect() {
	for (auto nextBlock : getNextBlocks()) {
		nextBlock->removeRefBlock(this);
	}
	m_nextNearBlock = m_nextFarBlock = nullptr;
}

std::list<PCode::Instruction*>& CE::Decompiler::PCodeBlock::getInstructions() {
	return m_instructions;
}

CE::ComplexOffset CE::Decompiler::PCodeBlock::getMinOffset() const
{
	return m_minOffset;
}

CE::ComplexOffset CE::Decompiler::PCodeBlock::getMaxOffset() const
{ // todo: auto-calculated?
	return m_maxOffset;
}

void CE::Decompiler::PCodeBlock::setMaxOffset(ComplexOffset offset) {
	m_maxOffset = offset;
}

void CE::Decompiler::PCodeBlock::removeNextBlock(PCodeBlock* nextBlock) {
	if (nextBlock == m_nextNearBlock)
		m_nextNearBlock = nullptr;
	if (nextBlock == m_nextFarBlock)
		m_nextFarBlock = nullptr;
}

void CE::Decompiler::PCodeBlock::setNextNearBlock(PCodeBlock* nextBlock) {
	m_nextNearBlock = nextBlock;
	nextBlock->m_blocksReferencedTo.push_back(this);
}

void CE::Decompiler::PCodeBlock::setNextFarBlock(PCodeBlock* nextBlock) {
	m_nextFarBlock = nextBlock;
	nextBlock->m_blocksReferencedTo.push_back(this);
}

PCodeBlock* CE::Decompiler::PCodeBlock::getNextNearBlock() const
{
	return m_nextNearBlock;
}

PCodeBlock* CE::Decompiler::PCodeBlock::getNextFarBlock() const
{
	return m_nextFarBlock;
}

std::list<PCodeBlock*> CE::Decompiler::PCodeBlock::getNextBlocks() const
{
	std::list<PCodeBlock*> nextBlocks;
	if (m_nextFarBlock) {
		nextBlocks.push_back(m_nextFarBlock);
	}
	if (m_nextNearBlock) {
		nextBlocks.push_back(m_nextNearBlock);
	}
	return nextBlocks;
}

PCode::Instruction* CE::Decompiler::PCodeBlock::getLastInstruction() {
	return *std::prev(m_instructions.end());
}

std::string CE::Decompiler::PCodeBlock::printDebug(void* addr, const std::string& tabStr, bool extraInfo, bool pcode) {
	std::string result;

	const ZyanU64 runtime_address = (ZyanU64)addr;
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
		result += "Level: " + std::to_string(m_level) + "\n";
		if (m_nextNearBlock != nullptr)
			result += "Next near: " + Helper::String::NumberToHex(m_nextNearBlock->getMinOffset()) + "\n";
		if (m_nextFarBlock != nullptr)
			result += "Next far: " + Helper::String::NumberToHex(m_nextFarBlock->getMinOffset()) + "\n";
	}

	return result;
}

CE::Decompiler::FunctionPCodeGraph::FunctionPCodeGraph(ImagePCodeGraph* imagePCodeGraph)
	: m_imagePCodeGraph(imagePCodeGraph)
{}

ImagePCodeGraph* CE::Decompiler::FunctionPCodeGraph::getImagePCodeGraph() const
{
	return m_imagePCodeGraph;
}

void CE::Decompiler::FunctionPCodeGraph::setStartBlock(PCodeBlock* block) {
	m_startBlock = block;
}

// head is a function that has not parents (main/all virtual functions)

bool CE::Decompiler::FunctionPCodeGraph::isHead() const
{
	return m_refFuncCalls.empty();
}

const std::set<FunctionPCodeGraph*>& FunctionPCodeGraph::getRefFuncCalls() const
{
	return m_refFuncCalls;
}

const std::set<FunctionPCodeGraph*>& FunctionPCodeGraph::getNonVirtFuncCalls() const
{
	return m_nonVirtFuncCalls;
}

const std::set<FunctionPCodeGraph*>& FunctionPCodeGraph::getVirtFuncCalls() const
{
	return m_virtFuncCalls;
}

void CE::Decompiler::FunctionPCodeGraph::addNonVirtFuncCall(FunctionPCodeGraph* funcGraph) {
	m_nonVirtFuncCalls.insert(funcGraph);
	funcGraph->m_refFuncCalls.insert(this);
}

void CE::Decompiler::FunctionPCodeGraph::addVirtFuncCall(FunctionPCodeGraph* funcGraph) {
	m_virtFuncCalls.insert(funcGraph);
	funcGraph->m_refFuncCalls.insert(this);
}

const std::set<PCodeBlock*>& CE::Decompiler::FunctionPCodeGraph::getBlocks() const
{
	return m_blocks;
}

void CE::Decompiler::FunctionPCodeGraph::addBlock(PCodeBlock* block) {
	m_blocks.insert(block);
	block->m_funcPCodeGraph = this;
}

PCodeBlock* CE::Decompiler::FunctionPCodeGraph::getStartBlock() const
{
	return m_startBlock;
}

std::map<PCode::Instruction*, PCode::DataValue>& CE::Decompiler::FunctionPCodeGraph::getConstValues() {
	return m_constValues;
}

void CE::Decompiler::FunctionPCodeGraph::printDebug(void* addr) {
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
