#pragma once
#include <Decompiler/PCode/Decompiler/PrimaryDecompiler.h>
#include <Decompiler/Optimization/DecGraphOptimization.h>

namespace CE::Decompiler
{
	// decompiler of pcode with optimization
	class Decompiler {
		FunctionPCodeGraph* m_funcGraph;
		PrimaryDecompiler::FuncCallInfoCallbackType m_funcCallInfoCallback;
		ReturnInfo m_retInfo;
		AbstractRegisterFactory* m_registerFactory;

		DecompiledCodeGraph* m_decompiledCodeGraph = nullptr;
	public:
		Decompiler(FunctionPCodeGraph* funcGraph, PrimaryDecompiler::FuncCallInfoCallbackType funcCallInfoCallback, ReturnInfo retInfo, AbstractRegisterFactory* registerFactory)
			: m_funcGraph(funcGraph), m_funcCallInfoCallback(funcCallInfoCallback), m_retInfo(retInfo), m_registerFactory(registerFactory)
		{}

		void start() {
			m_decompiledCodeGraph = new DecompiledCodeGraph(m_funcGraph);
			auto primaryDecompiler = PrimaryDecompiler(m_decompiledCodeGraph, m_registerFactory, m_retInfo, m_funcCallInfoCallback);
			primaryDecompiler.start();
			Optimization::ProcessDecompiledGraph(m_decompiledCodeGraph, &primaryDecompiler);
			m_decompiledCodeGraph->checkOnSingleParents();
		}

		DecompiledCodeGraph* getDecGraph() {
			return m_decompiledCodeGraph;
		}
	};
};