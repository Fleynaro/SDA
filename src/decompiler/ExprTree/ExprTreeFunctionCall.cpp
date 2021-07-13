#include "ExprTreeFunctionCall.h"

using namespace CE::Decompiler;
using namespace ExprTree;

FunctionCall::FunctionCall(INode* destination, PCode::Instruction* instr)
	: m_destination(destination), m_instr(instr)
{
	m_destination->addParentNode(this);
}

FunctionCall::~FunctionCall() {
	if (m_destination)
		m_destination->removeBy(this);
	for (auto paramNode : m_paramNodes) {
		paramNode->removeBy(this);
	}
}

void FunctionCall::replaceNode(INode* node, INode* newNode) {
	if (m_destination == node) {
		m_destination = newNode;
	}
	else {
		for (auto it = m_paramNodes.begin(); it != m_paramNodes.end(); it++) {
			if (node == *it) {
				*it = newNode;
			}
		}
	}
}

std::list<INode*> FunctionCall::getNodesList() {
	std::list<INode*> list = { m_destination };
	for (auto paramNode : m_paramNodes) {
		list.push_back(paramNode);
	}
	return list;
}

INode* FunctionCall::getDestination() const
{
	return m_destination;
}

std::vector<INode*>& FunctionCall::getParamNodes() {
	return m_paramNodes;
}

void FunctionCall::addParamNode(INode* node) {
	node->addParentNode(this);
	m_paramNodes.push_back(node);
}

int FunctionCall::getSize() {
	return m_functionResultVar ? m_functionResultVar->getSize() : 0x0;
}

bool FunctionCall::isFloatingPoint() {
	return false;
}

HS FunctionCall::getHash() {
	return m_functionResultVar ? m_functionResultVar->getHash() : m_destination->getHash();
}

std::list<PCode::Instruction*> FunctionCall::getInstructionsRelatedTo() {
	if (m_instr)
		return { m_instr };
	return {};
}

INode* FunctionCall::clone(NodeCloneContext* ctx) {
	const auto funcVar = m_functionResultVar ? dynamic_cast<Symbol::FunctionResultVar*>(m_functionResultVar->clone(ctx)) : nullptr;
	auto funcCallCtx = new FunctionCall(m_destination->clone(ctx), m_instr);
	funcCallCtx->m_functionResultVar = funcVar;
	for (auto paramNode : m_paramNodes) {
		funcCallCtx->addParamNode(paramNode->clone(ctx));
	}
	return funcCallCtx;
}

std::string FunctionCall::printDebug() {
	std::string str = "(" + getDestination()->printDebug() + ")(";
	for (auto paramNode : m_paramNodes) {
		str += paramNode->printDebug() + ", ";
	}
	if (!m_paramNodes.empty()) {
		str.pop_back();
		str.pop_back();
	}
	return (m_updateDebugInfo = (str + ")"));
}
