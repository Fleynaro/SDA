#pragma once
#include "ExprTreeFunctionCall.h"
#include "ExprTreeCondition.h"
#include "ExprTreeLinearExpr.h"
#include "ExprTreeAssignmentNode.h"
#include "ExprTreeMirrorNode.h"

namespace CE::Decompiler::ExprTree
{
	// get all symbol leafs from the specified {node} according to the specified {symbol}
	extern void GatherSymbolLeafsFromNode(INode* node, std::list<SymbolLeaf*>& symbolLeafs, Symbol::Symbol* symbol = nullptr);

	// check if the specified {node} has the specified {symbol}
	extern bool DoesNodeHaveSymbol(INode* node, Symbol::Symbol* symbol);

	// calculate a full mask for node
	extern ExprBitMask CalculateFullMask(INode* node);

	// calculate a mask for node
	extern ExprBitMask CalculateMask(INode* node);

	// get all pcode instructions the node depends on
	extern void GetInstructions(INode* node, std::list<PCode::Instruction*>& instructions);
};