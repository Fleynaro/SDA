#pragma once
#include "ExprModification.h"

namespace CE::Decompiler::Optimization
{
	//the very beigining optimization: 5*x -> x*5, 2x + (6y + 1) -> (6y + 1) + 2x
	class ExprUnification : public ExprModification
	{
	public:
		ExprUnification(INode* node);

		void start() override;
	private:
		// remove MirrorNode
		void processMirrorNode(MirrorNode* mirrorNode);

		// unify term order: left operand is expr, right operand is leaf (5*x -> x*5, x2 + (y6 + 1) -> (y6 + 1) + x2)
		void processOpNode(OperationalNode* opNode);

		// transform [rcx:8] to [rcx:8] & 0xFFFFFFFF because of the size(=4) defined in SymbolLeaf
		void processSymbolLeaf(SymbolLeaf* symbolLeaf);

		// leafs: x, x * 5, ...
		static bool IsLeaf(INode* node);
		
		static bool IsSwap(INode* leftNode, INode* rightNode);
	};
};