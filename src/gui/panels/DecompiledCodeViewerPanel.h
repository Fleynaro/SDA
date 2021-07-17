#pragma once
#include "decompiler/SDA/SdaCodeGraph.h"

namespace GUI
{
	class DecompiledCodeViewerPanel : public AbstractPanel
	{
		CE::Decompiler::SdaCodeGraph* m_sdaCodeGraph;
	public:
		DecompiledCodeViewerPanel(CE::Decompiler::SdaCodeGraph* sdaCodeGraph)
			: m_sdaCodeGraph(sdaCodeGraph)
		{}

		
	};
};