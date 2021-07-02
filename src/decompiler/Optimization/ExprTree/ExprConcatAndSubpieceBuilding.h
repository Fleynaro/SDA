#pragma once
#include "ExprModification.h"

namespace CE::Decompiler::Optimization
{
	//делать в самом конце. сюда же построение subpiece(если param1 - 8-байтовый объект). маску вычислять без кеша также. сделать linearexpr для логических операций.
	class ExprConcatAndSubpieceBuilding : public ExprModification
	{
	public:
		ExprConcatAndSubpieceBuilding(INode* node);

		void start() override;
	private:
		void dispatch(INode* node);

		void processOpNode(OperationalNode* opNode);

		static std::pair<INode*, int> GetConcatOperand(INode* node);
	};
};