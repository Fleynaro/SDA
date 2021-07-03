#include "DecCodeGraphBlock.h"

using namespace CE::Decompiler;

CE::Decompiler::EndDecBlock::EndDecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level)
	: DecBlock(decompiledGraph, pcodeBlock, level)
{
	m_returnNode = new ReturnTopNode(this);
}

CE::Decompiler::EndDecBlock::~EndDecBlock() {
	delete m_returnNode;
}

std::list<EndDecBlock::BlockTopNode*> CE::Decompiler::EndDecBlock::getAllTopNodes() {
	auto list = DecBlock::getAllTopNodes();
	if (getReturnNode()) {
		list.push_back(m_returnNode);
	}
	return list;
}

ExprTree::INode* CE::Decompiler::EndDecBlock::getReturnNode() const
{
	return m_returnNode->getNode();
}

void CE::Decompiler::EndDecBlock::setReturnNode(ExprTree::INode* returnNode) const
{
	if (getReturnNode()) {
		m_returnNode->clear();
	}
	m_returnNode->setNode(returnNode);
}

void CE::Decompiler::EndDecBlock::cloneAllExpr() {
	DecBlock::cloneAllExpr();
	if (getReturnNode())
		setReturnNode(getReturnNode()->clone());
}

CE::Decompiler::DecBlock::BlockTopNode::BlockTopNode(DecBlock* block, ExprTree::INode* node)
	: TopNode(node), m_block(block)
{}

CE::Decompiler::DecBlock::JumpTopNode::JumpTopNode(DecBlock* block)
	: BlockTopNode(block)
{}

ExprTree::AbstractCondition* CE::Decompiler::DecBlock::JumpTopNode::getCond() {
	return dynamic_cast<ExprTree::AbstractCondition*>(getNode());
}

void CE::Decompiler::DecBlock::JumpTopNode::setCond(ExprTree::AbstractCondition* cond) {
	setNode(cond);
}

CE::Decompiler::DecBlock::ReturnTopNode::ReturnTopNode(DecBlock* block)
	: BlockTopNode(block)
{}

CE::Decompiler::DecBlock::SeqAssignmentLine::SeqAssignmentLine(DecBlock* block, ExprTree::AssignmentNode* assignmentNode)
	: BlockTopNode(block, assignmentNode)
{}

CE::Decompiler::DecBlock::SeqAssignmentLine::SeqAssignmentLine(DecBlock * block, ExprTree::INode * dstNode, ExprTree::INode * srcNode, PCode::Instruction * instr)
	: SeqAssignmentLine(block, new ExprTree::AssignmentNode(dstNode, srcNode, instr))
{}

CE::Decompiler::DecBlock::SeqAssignmentLine::~SeqAssignmentLine() {
	m_block->m_seqLines.remove(this);
}

ExprTree::AssignmentNode* CE::Decompiler::DecBlock::SeqAssignmentLine::getAssignmentNode() {
	return dynamic_cast<ExprTree::AssignmentNode*>(getNode());
}

// left node from =

ExprTree::INode* CE::Decompiler::DecBlock::SeqAssignmentLine::getDstNode() {
	return getAssignmentNode()->getDstNode();
}

// right node from =

ExprTree::INode* CE::Decompiler::DecBlock::SeqAssignmentLine::getSrcNode() {
	return getAssignmentNode()->getSrcNode();
}

DecBlock::SeqAssignmentLine* CE::Decompiler::DecBlock::SeqAssignmentLine::clone(DecBlock* block, ExprTree::NodeCloneContext* ctx) {
	return new SeqAssignmentLine(block, dynamic_cast<ExprTree::AssignmentNode*>(getNode()->clone(ctx)));
}

CE::Decompiler::DecBlock::SymbolParallelAssignmentLine::SymbolParallelAssignmentLine(DecBlock* block, ExprTree::AssignmentNode* assignmentNode)
	: SeqAssignmentLine(block, assignmentNode)
{}

CE::Decompiler::DecBlock::SymbolParallelAssignmentLine::SymbolParallelAssignmentLine(DecBlock * block, ExprTree::SymbolLeaf * dstNode, ExprTree::INode * srcNode, PCode::Instruction * instr)
	: SeqAssignmentLine(block, dstNode, srcNode, instr)
{}

CE::Decompiler::DecBlock::SymbolParallelAssignmentLine::~SymbolParallelAssignmentLine() {
	m_block->m_symbolParallelAssignmentLines.remove(this);
}

ExprTree::SymbolLeaf* CE::Decompiler::DecBlock::SymbolParallelAssignmentLine::getDstSymbolLeaf() {
	return dynamic_cast<ExprTree::SymbolLeaf*>(getDstNode());
}

DecBlock::SymbolParallelAssignmentLine* CE::Decompiler::DecBlock::SymbolParallelAssignmentLine::clone(DecBlock* block, ExprTree::NodeCloneContext* ctx) {
	return new SymbolParallelAssignmentLine(block, dynamic_cast<ExprTree::AssignmentNode*>(getNode()->clone(ctx)));
}

CE::Decompiler::DecBlock::DecBlock(DecompiledCodeGraph* decompiledGraph, PCodeBlock* pcodeBlock, int level)
	: m_level(level), m_pcodeBlock(pcodeBlock), m_decompiledGraph(decompiledGraph)
{
	m_noJmpCond = new JumpTopNode(this);
}

CE::Decompiler::DecBlock::~DecBlock() {
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

void CE::Decompiler::DecBlock::clearCode() {
	for (auto line : m_seqLines) {
		delete line;
	}
	m_seqLines.clear();
	m_noJmpCond->clear();
}

// make the block independent from the decompiled graph

void CE::Decompiler::DecBlock::disconnect() {
	for (auto nextBlock : getNextBlocks()) {
		nextBlock->removeRefBlock(this);
	}
	m_nextNearBlock = m_nextFarBlock = nullptr;
}

void CE::Decompiler::DecBlock::removeRefBlock(DecBlock* block) {
	m_blocksReferencedTo.remove(block);
}

void CE::Decompiler::DecBlock::setNextNearBlock(DecBlock* nextBlock) {
	if (nextBlock) {
		nextBlock->removeRefBlock(m_nextNearBlock);
		nextBlock->m_blocksReferencedTo.push_back(this);
	}
	m_nextNearBlock = nextBlock;
}

void CE::Decompiler::DecBlock::setNextFarBlock(DecBlock* nextBlock) {
	if (nextBlock) {
		nextBlock->removeRefBlock(m_nextFarBlock);
		nextBlock->m_blocksReferencedTo.push_back(this);
	}
	m_nextFarBlock = nextBlock;
}

DecBlock* CE::Decompiler::DecBlock::getNextNearBlock() const
{
	return m_nextNearBlock;
}

DecBlock* CE::Decompiler::DecBlock::getNextFarBlock() const
{
	return m_nextFarBlock;
}

std::list<DecBlock*>& CE::Decompiler::DecBlock::getBlocksReferencedTo() {
	return m_blocksReferencedTo;
}

std::list<DecBlock*> CE::Decompiler::DecBlock::getNextBlocks() const
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

DecBlock* CE::Decompiler::DecBlock::getNextBlock() const
{
	if (m_nextFarBlock) {
		return m_nextFarBlock;
	}
	if (m_nextNearBlock) {
		return m_nextNearBlock;
	}
	return nullptr;
}

void CE::Decompiler::DecBlock::swapNextBlocks() {
	std::swap(m_nextNearBlock, m_nextFarBlock);
}

bool CE::Decompiler::DecBlock::isCondition() const
{
	return m_nextNearBlock != nullptr && m_nextFarBlock != nullptr;
}

bool CE::Decompiler::DecBlock::isCycle() {
	return (int)m_blocksReferencedTo.size() != getRefHighBlocksCount();
}

// get count of blocks which reference to this block

int CE::Decompiler::DecBlock::getRefBlocksCount() const
{
	return (int)m_blocksReferencedTo.size();
}

// get count of blocks which reference to this block without loops

int CE::Decompiler::DecBlock::getRefHighBlocksCount() {
	int count = 0;
	for (auto refBlock : m_blocksReferencedTo) {
		if (refBlock->m_level < m_level)
			count++;
	}
	return count;
}

// get all top nodes for this block (assignments, function calls, return) / get all expressions

std::list<DecBlock::BlockTopNode*> CE::Decompiler::DecBlock::getAllTopNodes() {
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

// condition top node which contains boolean expression to jump to another block

ExprTree::AbstractCondition* CE::Decompiler::DecBlock::getNoJumpCondition() const
{
	return m_noJmpCond->getCond();
}

void CE::Decompiler::DecBlock::setNoJumpCondition(ExprTree::AbstractCondition* noJmpCond) const
{
	if (getNoJumpCondition()) {
		m_noJmpCond->clear();
	}
	if (noJmpCond) {
		m_noJmpCond->setNode(noJmpCond);
	}
}

void CE::Decompiler::DecBlock::addSeqLine(ExprTree::INode* destAddr, ExprTree::INode* srcValue, PCode::Instruction* instr) {
	m_seqLines.push_back(new SeqAssignmentLine(this, destAddr, srcValue, instr));
}

std::list<DecBlock::SeqAssignmentLine*>& CE::Decompiler::DecBlock::getSeqAssignmentLines() {
	return m_seqLines;
}

void CE::Decompiler::DecBlock::addSymbolParallelAssignmentLine(ExprTree::SymbolLeaf* symbolLeaf, ExprTree::INode* srcValue, PCode::Instruction* instr) {
	m_symbolParallelAssignmentLines.push_back(new SymbolParallelAssignmentLine(this, symbolLeaf, srcValue, instr));
}

std::list<DecBlock::SymbolParallelAssignmentLine*>& CE::Decompiler::DecBlock::getSymbolParallelAssignmentLines() {
	return m_symbolParallelAssignmentLines;
}

// check if this block is empty

bool CE::Decompiler::DecBlock::hasNoCode() const
{
	return m_seqLines.empty() && m_symbolParallelAssignmentLines.empty();
}

// clone all expr.

void CE::Decompiler::DecBlock::cloneAllExpr() {
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

std::string CE::Decompiler::DecBlock::printDebug(bool cond, const std::string& tabStr) {
	std::string result;
	for (auto line : m_seqLines) {
		result += tabStr + line->getNode()->printDebug();
	}
	if (!m_symbolParallelAssignmentLines.empty())
		result += tabStr + "<Symbol assignments>:\n";
	for (auto line : m_symbolParallelAssignmentLines) {
		result += tabStr + "- " + line->getNode()->printDebug();
	}
	if (cond && getNoJumpCondition() != nullptr) {
		result += "------> Condition: " + getNoJumpCondition()->printDebug() + "\n";
	}
	return result;
}
