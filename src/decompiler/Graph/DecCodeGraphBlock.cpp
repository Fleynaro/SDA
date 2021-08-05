#include "DecCodeGraphBlock.h"

using namespace CE::Decompiler;

EndDecBlock::EndDecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level)
	: DecBlock(decompiledGraph, pcodeBlock, level)
{
	m_returnNode = new ReturnTopNode(this);
}

EndDecBlock::~EndDecBlock() {
	delete m_returnNode;
}

std::list<EndDecBlock::BlockTopNode*> EndDecBlock::getAllTopNodes() {
	auto list = DecBlock::getAllTopNodes();
	if (getReturnTopNode()->getNode()) {
		list.push_back(m_returnNode);
	}
	return list;
}

DecBlock::ReturnTopNode* EndDecBlock::getReturnTopNode() const {
	return m_returnNode;
}

void EndDecBlock::setReturnNode(ExprTree::INode* returnNode) const
{
	if (getReturnTopNode()->getNode()) {
		m_returnNode->clear();
	}
	m_returnNode->setNode(returnNode);
}

void EndDecBlock::cloneAllExpr() {
	DecBlock::cloneAllExpr();
	if (getReturnTopNode()->getNode())
		setReturnNode(getReturnTopNode()->getNode()->clone());
}

DecBlock::BlockTopNode::BlockTopNode(DecBlock* block, ExprTree::INode* node)
	: TopNode(node), m_block(block)
{}

Instruction* DecBlock::BlockTopNode::getLastReqInstr() {
	return m_block->m_pcodeBlock->getLastInstruction();
}

DecBlock::JumpTopNode::JumpTopNode(DecBlock* block)
	: BlockTopNode(block)
{}

ExprTree::AbstractCondition* DecBlock::JumpTopNode::getCond() {
	return dynamic_cast<ExprTree::AbstractCondition*>(getNode());
}

void DecBlock::JumpTopNode::setCond(ExprTree::AbstractCondition* cond) {
	setNode(cond);
}

DecBlock::ReturnTopNode::ReturnTopNode(DecBlock* block)
	: BlockTopNode(block)
{}

DecBlock::SeqAssignmentLine::SeqAssignmentLine(DecBlock* block, ExprTree::AssignmentNode* assignmentNode)
	: BlockTopNode(block, assignmentNode)
{}

DecBlock::SeqAssignmentLine::SeqAssignmentLine(DecBlock * block, ExprTree::INode * dstNode, ExprTree::INode * srcNode, PCode::Instruction * instr)
	: SeqAssignmentLine(block, new ExprTree::AssignmentNode(dstNode, srcNode, instr))
{}

DecBlock::SeqAssignmentLine::~SeqAssignmentLine() {
	m_block->m_seqLines.remove(this);
}

ExprTree::AssignmentNode* DecBlock::SeqAssignmentLine::getAssignmentNode() {
	return dynamic_cast<ExprTree::AssignmentNode*>(getNode());
}

// left node from =

ExprTree::INode* DecBlock::SeqAssignmentLine::getDstNode() {
	return getAssignmentNode()->getDstNode();
}

// right node from =

ExprTree::INode* DecBlock::SeqAssignmentLine::getSrcNode() {
	return getAssignmentNode()->getSrcNode();
}

Instruction* DecBlock::SeqAssignmentLine::getLastReqInstr() {
	return m_lastRequiredInstruction;
}

DecBlock::SeqAssignmentLine* DecBlock::SeqAssignmentLine::clone(DecBlock* block, ExprTree::NodeCloneContext* ctx) {
	return new SeqAssignmentLine(block, dynamic_cast<ExprTree::AssignmentNode*>(getNode()->clone(ctx)));
}

DecBlock::SymbolParallelAssignmentLine::SymbolParallelAssignmentLine(DecBlock* block, ExprTree::AssignmentNode* assignmentNode)
	: SeqAssignmentLine(block, assignmentNode)
{}

DecBlock::SymbolParallelAssignmentLine::SymbolParallelAssignmentLine(DecBlock * block, ExprTree::SymbolLeaf * dstNode, ExprTree::INode * srcNode, PCode::Instruction * instr)
	: SeqAssignmentLine(block, dstNode, srcNode, instr)
{}

DecBlock::SymbolParallelAssignmentLine::~SymbolParallelAssignmentLine() {
	m_block->m_symbolParallelAssignmentLines.remove(this);
}

ExprTree::SymbolLeaf* DecBlock::SymbolParallelAssignmentLine::getDstSymbolLeaf() {
	return dynamic_cast<ExprTree::SymbolLeaf*>(getDstNode());
}

DecBlock::SymbolParallelAssignmentLine* DecBlock::SymbolParallelAssignmentLine::clone(DecBlock* block, ExprTree::NodeCloneContext* ctx) {
	return new SymbolParallelAssignmentLine(block, dynamic_cast<ExprTree::AssignmentNode*>(getNode()->clone(ctx)));
}

DecBlock::DecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level)
	: m_level(level), m_pcodeBlock(pcodeBlock), m_decompiledGraph(decompiledGraph)
{
	m_noJmpCond = new JumpTopNode(this);
}

DecBlock::~DecBlock() {
	clearCode();

	for (auto line : m_symbolParallelAssignmentLines) {
		delete line;
	}

	for (auto refBlock : m_blocksReferencedTo) {
		if (refBlock->m_nextNearBlock == this)
			refBlock->m_nextNearBlock = refBlock->m_nextFarBlock;
		else if (refBlock->m_nextFarBlock == this)
			refBlock->m_nextFarBlock = nullptr;
	}

	delete m_noJmpCond;

	disconnect();
}

void DecBlock::clearCode() {
	auto it = m_seqLines.begin();
	while (it != m_seqLines.end()) {
		const auto line = *it;
		++it;
		delete line;
	}
	m_seqLines.clear();
	m_noJmpCond->clear();
}

// make the block independent from the decompiled graph

void DecBlock::disconnect() {
	for (auto nextBlock : getNextBlocks()) {
		nextBlock->removeRefBlock(this);
	}
	m_nextNearBlock = m_nextFarBlock = nullptr;
}

void DecBlock::removeRefBlock(DecBlock* block) {
	m_blocksReferencedTo.remove(block);
}

void DecBlock::setNextNearBlock(DecBlock* nextBlock) {
	if (nextBlock) {
		nextBlock->removeRefBlock(m_nextNearBlock);
		nextBlock->m_blocksReferencedTo.push_back(this);
	}
	m_nextNearBlock = nextBlock;
}

void DecBlock::setNextFarBlock(DecBlock* nextBlock) {
	if (nextBlock) {
		nextBlock->removeRefBlock(m_nextFarBlock);
		nextBlock->m_blocksReferencedTo.push_back(this);
	}
	m_nextFarBlock = nextBlock;
}

DecBlock* DecBlock::getNextNearBlock() const
{
	return m_nextNearBlock;
}

DecBlock* DecBlock::getNextFarBlock() const
{
	return m_nextFarBlock;
}

std::list<DecBlock*>& DecBlock::getBlocksReferencedTo() {
	return m_blocksReferencedTo;
}

std::list<DecBlock*> DecBlock::getNextBlocks() const
{
	std::list<DecBlock*> nextBlocks;
	if (m_nextFarBlock) {
		nextBlocks.push_back(m_nextFarBlock);
	}
	if (m_nextNearBlock) {
		nextBlocks.push_back(m_nextNearBlock);
	}
	return nextBlocks;
}

DecBlock* DecBlock::getNextBlock() const
{
	if (m_nextFarBlock) {
		return m_nextFarBlock;
	}
	if (m_nextNearBlock) {
		return m_nextNearBlock;
	}
	return nullptr;
}

void DecBlock::swapNextBlocks() {
	std::swap(m_nextNearBlock, m_nextFarBlock);
}

bool DecBlock::isCondition() const
{
	return m_nextNearBlock != nullptr && m_nextFarBlock != nullptr;
}

bool DecBlock::isCycle() {
	return static_cast<int>(m_blocksReferencedTo.size()) != getRefHighBlocksCount();
}

// get count of blocks which reference to this block

int DecBlock::getRefBlocksCount() const
{
	return static_cast<int>(m_blocksReferencedTo.size());
}

// get count of blocks which reference to this block without loops

int DecBlock::getRefHighBlocksCount() {
	int count = 0;
	for (auto refBlock : m_blocksReferencedTo) {
		if (refBlock->m_level < m_level)
			count++;
	}
	return count;
}

// get all top nodes for this block (assignments, function calls, return) / get all expressions

std::list<DecBlock::BlockTopNode*> DecBlock::getAllTopNodes() {
	std::list<BlockTopNode*> result;
	for (auto line : getSeqAssignmentLines()) {
		result.push_back(line);
	}
	for (auto line : getSymbolParallelAssignmentLines()) {
		result.push_back(line);
	}

	if (getNoJumpCondition())
		result.push_back(m_noJmpCond);
	return result;
}

DecBlock::JumpTopNode* DecBlock::getJumpTopNode() const {
	return m_noJmpCond;
}

// condition top node which contains boolean expression to jump to another block

ExprTree::AbstractCondition* DecBlock::getNoJumpCondition() const
{
	return m_noJmpCond->getCond();
}

void DecBlock::setNoJumpCondition(ExprTree::AbstractCondition* noJmpCond) const
{
	if (getNoJumpCondition()) {
		m_noJmpCond->clear();
	}
	if (noJmpCond) {
		m_noJmpCond->setNode(noJmpCond);
	}
}

void DecBlock::addSeqLine(ExprTree::INode* destAddr, ExprTree::INode* srcValue, PCode::Instruction* instr) {
	m_seqLines.push_back(new SeqAssignmentLine(this, destAddr, srcValue, instr));
}

std::list<DecBlock::SeqAssignmentLine*>& DecBlock::getSeqAssignmentLines() {
	return m_seqLines;
}

void DecBlock::addSymbolParallelAssignmentLine(ExprTree::SymbolLeaf* symbolLeaf, ExprTree::INode* srcValue, PCode::Instruction* instr) {
	m_symbolParallelAssignmentLines.push_back(new SymbolParallelAssignmentLine(this, symbolLeaf, srcValue, instr));
}

std::list<DecBlock::SymbolParallelAssignmentLine*>& DecBlock::getSymbolParallelAssignmentLines() {
	return m_symbolParallelAssignmentLines;
}

DecBlock::BlockTopNode* DecBlock::findBlockTopNodeByOffset(ComplexOffset offset) {
	const auto allTopNodes = getAllTopNodes();
	for (const auto blockTopNode : allTopNodes) {
		if (!blockTopNode->getLastReqInstr())
			continue;
		if (offset <= blockTopNode->getLastReqInstr()->getOffset())
			return blockTopNode;
	}
	if (allTopNodes.empty())
		return nullptr;
	return *allTopNodes.rbegin();
}

// check if this block is empty

bool DecBlock::hasNoCode() const
{
	return m_seqLines.empty() && m_symbolParallelAssignmentLines.empty();
}

// clone all expr.

void DecBlock::cloneAllExpr() {
	ExprTree::NodeCloneContext nodeCloneContext;

	auto seqLines = m_seqLines;
	m_seqLines.clear();
	for (auto line : seqLines) {
		m_seqLines.push_back(line->clone(this, &nodeCloneContext));
		delete line;
	}

	auto symbolParallelAssignmentLines = m_symbolParallelAssignmentLines;
	m_symbolParallelAssignmentLines.clear();
	for (auto line : symbolParallelAssignmentLines) {
		m_symbolParallelAssignmentLines.push_back(line->clone(this, &nodeCloneContext));
		delete line;
	}

	if (getNoJumpCondition())
		setNoJumpCondition(dynamic_cast<ExprTree::AbstractCondition*>(getNoJumpCondition()->clone(&nodeCloneContext)));
}