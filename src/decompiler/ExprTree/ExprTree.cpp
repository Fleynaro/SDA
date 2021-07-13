#include "ExprTree.h"

using namespace CE::Decompiler;
using namespace ExprTree;

// get all symbol leafs from the specified {node} according to the specified {symbol}
void ExprTree::GatherSymbolLeafsFromNode(INode* node, std::list<SymbolLeaf*>& symbolLeafs, Symbol::Symbol* symbol) {
	node->iterateChildNodes([&](INode* childNode) {
		GatherSymbolLeafsFromNode(childNode, symbolLeafs, symbol);
		});

	if (const auto symbolLeaf = dynamic_cast<SymbolLeaf*>(node)) {
		if (!symbol || symbolLeaf->m_symbol == symbol) {
			symbolLeafs.push_back(symbolLeaf);
		}
	}
}

// check if the specified {node} has the specified {symbol}
bool ExprTree::DoesNodeHaveSymbol(INode* node, Symbol::Symbol* symbol) {
	std::list<SymbolLeaf*> symbolLeafs;
	GatherSymbolLeafsFromNode(node, symbolLeafs, symbol);
	return !symbolLeafs.empty();
}

// calculate a full mask for node
ExprBitMask ExprTree::CalculateFullMask(INode* node) {
	if (auto numberLeaf = dynamic_cast<INumberLeaf*>(node))
		return numberLeaf->getValue();

	if (const auto opNode = dynamic_cast<OperationalNode*>(node))
	{
		if (opNode->m_rightNode)
		{
			if (opNode->m_operation == And)
				return CalculateFullMask(opNode->m_leftNode) & CalculateFullMask(opNode->m_rightNode);

			if (opNode->m_operation == Shl || opNode->m_operation == Shr) {
				if (auto numberLeaf = dynamic_cast<INumberLeaf*>(opNode->m_rightNode)) {
					if (opNode->m_operation == Shl) {
						return CalculateFullMask(opNode->m_leftNode) << static_cast<int>(numberLeaf->getValue());
					}
					else {
						if (numberLeaf->getValue() == 64)
							return static_cast<uint64_t>(0);
						return CalculateFullMask(opNode->m_leftNode) >> static_cast<int>(numberLeaf->getValue());
					}
				}

				// todo: see the case ([rcx] & 0x5123) << [rdx]
				if (!opNode->m_instr) {
					if (opNode->m_operation == Shl)
						return ~BitMask64::GetBitMask64BySizeInBits(CalculateFullMask(opNode->m_rightNode).getOffset());
					return BitMask64::GetBitMask64BySizeInBits(CalculateFullMask(opNode->m_leftNode).getSizeInBits());
				}
			}
		}

		if (GetOperationGroup(opNode->m_operation) == OperationGroup::Arithmetic) {
			uint64_t maxValue = 0;
			switch (opNode->m_operation)
			{
			case Add:
			case Mul:
			{
				const auto m1 = CalculateFullMask(opNode->m_leftNode).getValue();
				const auto m2 = CalculateFullMask(opNode->m_rightNode).getValue();
				if (opNode->m_operation == Add) {
					maxValue = m1 | m2 | (m1 + m2);
				}
				else {
					maxValue = m1 * m2;
					if (m1 != 0 && maxValue / m1 != m2) {
						maxValue = static_cast<int64_t>(-1);
					}
				}
				break;
			}
			case Div:
				maxValue = CalculateFullMask(opNode->m_leftNode).getValue();
				break;
			case Mod:
				maxValue = CalculateFullMask(opNode->m_rightNode).getValue();
				break;
			}
			return BitMask64::GetBitMask64BySizeInBits(ExprBitMask(maxValue).getSizeInBits());
		}
	}

	return ExprBitMask(node->getSize());
}

// calculate a mask for node
ExprBitMask ExprTree::CalculateMask(INode* node) {
	return CalculateFullMask(node) & ExprBitMask(node->getSize());
}
