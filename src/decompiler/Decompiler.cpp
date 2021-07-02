#include "Decompiler.h"

using namespace CE::Decompiler;

void CE::Decompiler::Decompiler::start() {
	m_decompiledCodeGraph = new DecompiledCodeGraph(m_funcGraph);
	auto primaryDecompiler = PrimaryDecompiler(m_decompiledCodeGraph, m_registerFactory, m_retInfo, m_funcCallInfoCallback);
	primaryDecompiler.start();
	Optimization::ProcessDecompiledGraph(m_decompiledCodeGraph, &primaryDecompiler);
	m_decompiledCodeGraph->checkOnSingleParents();
}

DecompiledCodeGraph* CE::Decompiler::Decompiler::getDecGraph() {
	return m_decompiledCodeGraph;
}
