#pragma once
#include <decompiler/PCode/Decompiler/PrimaryDecompiler.h>
#include <decompiler/Optimization/DecGraphOptimization.h>

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

		void start();

		DecompiledCodeGraph* getDecGraph();
	};
};