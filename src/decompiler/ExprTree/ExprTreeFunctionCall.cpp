#include "ExprTreeFunctionCall.h"

using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;

CE::Decompiler::ExprTree::FunctionCall::FunctionCall(INode* destination, PCode::Instruction* instr)
	: m_destination(destination), m_instr(instr)
{
	m_destination->addParentNode(this);
}

CE::Decompiler::ExprTree::FunctionCall::~FunctionCall() {
	if (m_destination)
		m_destination->removeBy(this);
	for (auto paramNode : m_paramNodes) {
		paramNode->removeBy(this);
	}
}

void CE::Decompiler::ExprTree::FunctionCall::replaceNode(INode* node, INode* newNode) {
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

std::list<ExprTree::INode*> CE::Decompiler::ExprTree::FunctionCall::getNodesList() {
	std::list<ExprTree::INode*> list = { m_destination };
	for (auto paramNode : m_paramNodes) {
		list.push_back(paramNode);
	}
	return list;
}

INode* CE::Decompiler::ExprTree::FunctionCall::getDestination() {
	return m_destination;
}

std::vector<INode*>& CE::Decompiler::ExprTree::FunctionCall::getParamNodes() {
	return m_paramNodes;
}

void CE::Decompiler::ExprTree::FunctionCall::addParamNode(INode* node) {
	node->addParentNode(this);
	m_paramNodes.push_back(node);
}

int CE::Decompiler::ExprTree::FunctionCall::getSize() {
	return m_functionResultVar ? m_functionResultVar->getSize() : 0x0;
}

bool CE::Decompiler::ExprTree::FunctionCall::isFloatingPoint() {
	return false;
}

HS CE::Decompiler::ExprTree::FunctionCall::getHash() {
	return m_functionResultVar ? m_functionResultVar->getHash() : m_destination->getHash();
}

std::list<PCode::Instruction*> CE::Decompiler::ExprTree::FunctionCall::getInstructionsRelatedTo() {
	if (m_instr)
		return { m_instr };
	return {};
}

INode* CE::Decompiler::ExprTree::FunctionCall::clone(NodeCloneContext* ctx) {
	auto funcVar = m_functionResultVar ? dynamic_cast<Symbol::FunctionResultVar*>(m_functionResultVar->clone(ctx)) : nullptr;
	auto funcCallCtx = new FunctionCall(m_destination->clone(ctx), m_instr);
	funcCallCtx->m_functionResultVar = funcVar;
	for (auto paramNode : m_paramNodes) {
		funcCallCtx->addParamNode(paramNode->clone(ctx));
	}
	return funcCallCtx;
}

std::string CE::Decompiler::ExprTree::FunctionCall::printDebug() {
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
