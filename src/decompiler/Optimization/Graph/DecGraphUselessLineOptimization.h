#pragma once
#include "DecGraphModification.h"

namespace CE::Decompiler::Optimization
{
	// Removing seq. assignments lines with unknown register
	class GraphUselessLineDeleting : public GraphModification
	{
	public:
		GraphUselessLineDeleting(DecompiledCodeGraph* decGraph);

		void start() override;
	private:
		void processBlock(DecBlock* block);

		bool hasUnknownRegister(INode* node);

		bool isUnknownRegister(Register& reg);
	};
};