#pragma once
#include "../../Graph/DecCodeGraph.h"

namespace CE::Decompiler
{
	using namespace ExprTree;

	// abstract class for some graph modification
	class GraphModification
	{
	public:
		GraphModification(DecompiledCodeGraph* decGraph);

		// here creating of modification logic
		virtual void start() = 0;

	protected:
		DecompiledCodeGraph* m_decGraph;

		// iterate over all top nodes of the dec. graph (allow to access all expressions)
		void passAllTopNodes(std::function<void(DecBlock::BlockTopNode*)> func) const;
	};
};