#pragma once
#include "DecGraphModification.h"
#include <decompiler/PCode/Decompiler/PrimaryDecompiler.h>
#include <decompiler/Optimization/ExprOptimization.h>

namespace CE::Decompiler::Optimization
{
	// Iterate over all exprs in dec. graph and find localVar to create par. assignments
	class GraphParAssignmentCreator : public GraphModification
	{
		PrimaryDecompiler* m_decompiler;

		struct LocalVarInfo {
			bool areAllParentOpNode = true;
			std::list<std::pair<ExprTree::OperationalNode*, bool>> m_opNodes;
		};
		std::map<Symbol::LocalVariable*, LocalVarInfo> m_localVars;
	public:
		GraphParAssignmentCreator(DecompiledCodeGraph* decGraph, PrimaryDecompiler* decompiler);

		void start() override;
	private:
		
		void findAllLocalVarsAndGatherParentOpNodes(DecBlock::BlockTopNode* topNode);

		void createParAssignmentsForLocalVars();

		// optimize: localVar = ((5 << 32) | 10) & 0xFFFFFFFF -> localVar = 10
		void optimizeAllParAssignments() const;
	};
};