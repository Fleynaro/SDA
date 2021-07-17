#include "DecPCodeGraph.h"
#include <utilities/Helper.h>

using namespace CE::Decompiler;

// add all head functions into the list HeadFuncGraphs

ImagePCodeGraph::ImagePCodeGraph()
{}

FunctionPCodeGraph* ImagePCodeGraph::createFunctionGraph() {
	m_funcGraphList.emplace_back(this);
	return &*m_funcGraphList.rbegin();
}

PCodeBlock* ImagePCodeGraph::createBlock(ComplexOffset minOffset, ComplexOffset maxOffset) {
	m_blocks.insert(std::make_pair(minOffset, PCodeBlock(minOffset, maxOffset)));
	const auto newBlock = &m_blocks[minOffset];
	return newBlock;
}

PCodeBlock* ImagePCodeGraph::createBlock(ComplexOffset offset) {
	return createBlock(offset, offset);
}

const std::list<FunctionPCodeGraph*>& ImagePCodeGraph::getHeadFuncGraphs() const
{
	return m_headFuncGraphs;
}

std::list<FunctionPCodeGraph>& ImagePCodeGraph::getFunctionGraphList() {
	return m_funcGraphList;
}

FunctionPCodeGraph* ImagePCodeGraph::getEntryFunctionGraph() {
	return &*m_funcGraphList.begin();
}

PCodeBlock* ImagePCodeGraph::getBlockAtOffset(ComplexOffset offset, bool halfInterval) {
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

FunctionPCodeGraph* ImagePCodeGraph::getFuncGraphAt(ComplexOffset offset, bool halfInterval) {
	const auto block = getBlockAtOffset(offset, halfInterval);
	return block->m_funcPCodeGraph;
}

void ImagePCodeGraph::fillHeadFuncGraphs() {
	for (auto& funcGraph : getFunctionGraphList()) {
		if (funcGraph.isHead())
			m_headFuncGraphs.push_back(&funcGraph);
	}
}

PCodeBlock::PCodeBlock(ComplexOffset minOffset, ComplexOffset maxOffset)
	: m_minOffset(minOffset), m_maxOffset(maxOffset), ID(static_cast<int>(minOffset >> 8))
{}

void PCodeBlock::removeRefBlock(PCodeBlock * block) {
	m_blocksReferencedTo.remove(block);
}

void PCodeBlock::disconnect() {
	for (auto nextBlock : getNextBlocks()) {
		nextBlock->removeRefBlock(this);
	}
	m_nextNearBlock = m_nextFarBlock = nullptr;
}

std::list<PCodeBlock*> PCodeBlock::getRefHighBlocks() const {
	std::list<PCodeBlock*> blocks;
	for (auto refBlock : m_blocksReferencedTo) {
		if (refBlock->m_level < m_level)
			blocks.push_back(refBlock);
	}
	return blocks;
}

std::list<Instruction*>& PCodeBlock::getInstructions() {
	return m_instructions;
}

CE::ComplexOffset PCodeBlock::getMinOffset() const
{
	return m_minOffset;
}

CE::ComplexOffset PCodeBlock::getMaxOffset() const
{ // todo: auto-calculated?
	return m_maxOffset;
}

void PCodeBlock::setMaxOffset(ComplexOffset offset) {
	m_maxOffset = offset;
}

void PCodeBlock::removeNextBlock(PCodeBlock* nextBlock) {
	if (nextBlock == m_nextNearBlock)
		m_nextNearBlock = nullptr;
	if (nextBlock == m_nextFarBlock)
		m_nextFarBlock = nullptr;
}

void PCodeBlock::setNextNearBlock(PCodeBlock* nextBlock) {
	m_nextNearBlock = nextBlock;
	nextBlock->m_blocksReferencedTo.push_back(this);
}

void PCodeBlock::setNextFarBlock(PCodeBlock* nextBlock) {
	m_nextFarBlock = nextBlock;
	nextBlock->m_blocksReferencedTo.push_back(this);
}

PCodeBlock* PCodeBlock::getNextNearBlock() const
{
	return m_nextNearBlock;
}

PCodeBlock* PCodeBlock::getNextFarBlock() const
{
	return m_nextFarBlock;
}

std::list<PCodeBlock*> PCodeBlock::getNextBlocks() const
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

Instruction* PCodeBlock::getLastInstruction() {
	return *std::prev(m_instructions.end());
}

FunctionPCodeGraph::FunctionPCodeGraph(ImagePCodeGraph* imagePCodeGraph)
	: m_imagePCodeGraph(imagePCodeGraph)
{}

ImagePCodeGraph* FunctionPCodeGraph::getImagePCodeGraph() const
{
	return m_imagePCodeGraph;
}

void FunctionPCodeGraph::setStartBlock(PCodeBlock* block) {
	m_startBlock = block;
}

// head is a function that has not parents (main/all virtual functions)

bool FunctionPCodeGraph::isHead() const
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

void FunctionPCodeGraph::addNonVirtFuncCall(FunctionPCodeGraph* funcGraph) {
	m_nonVirtFuncCalls.insert(funcGraph);
	funcGraph->m_refFuncCalls.insert(this);
}

void FunctionPCodeGraph::addVirtFuncCall(FunctionPCodeGraph* funcGraph) {
	m_virtFuncCalls.insert(funcGraph);
	funcGraph->m_refFuncCalls.insert(this);
}

const std::set<PCodeBlock*>& FunctionPCodeGraph::getBlocks() const
{
	return m_blocks;
}

void FunctionPCodeGraph::addBlock(PCodeBlock* block) {
	m_blocks.insert(block);
	block->m_funcPCodeGraph = this;
}

PCodeBlock* FunctionPCodeGraph::getStartBlock() const
{
	return m_startBlock;
}

std::map<Instruction*, DataValue>& FunctionPCodeGraph::getConstValues() {
	return m_constValues;
}