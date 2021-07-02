#pragma once
#include "ExprModification.h"

namespace CE::Decompiler::Optimization
{
	//������ � ����� �����. ���� �� ���������� subpiece(���� param1 - 8-�������� ������). ����� ��������� ��� ���� �����. ������� linearexpr ��� ���������� ��������.
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