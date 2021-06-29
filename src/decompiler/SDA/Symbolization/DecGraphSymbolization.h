#pragma once
#include "DecGraphSdaBuilding.h"
#include "SdaGraphDataTypeCalc.h"

namespace CE::Decompiler::Symbolization
{
	static void SymbolizeWithSDA(SdaCodeGraph* sdaCodeGraph, SymbolContext& symbolContext, Project* project) {
		SdaBuilding sdaBuilding(sdaCodeGraph, &symbolContext, project);
		sdaBuilding.start();

		SdaDataTypesCalculater sdaDataTypesCalculating(sdaCodeGraph, symbolContext.m_signature, project);
		sdaDataTypesCalculating.start();
	}
};