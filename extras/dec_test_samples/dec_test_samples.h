#pragma once
#include "decompiler/Graph/DecCodeGraph.h"
#include "decompiler/SDA/SdaCodeGraph.h"
#include "images/VectorBufferImage.h"
#include "managers/Managers.h"
#include <string>

namespace CE
{
	class DecTestSamplesPool
	{
		Project* m_project;
		DataType::FunctionSignature* m_defSignature;
		DataType::Structure* m_vec3D;
		DataType::Structure* m_vecExt3D;
		DataType::Structure* m_vec4D;
		DataType::Structure* m_matrix4x4;
		AddressSpace* m_addrSpace;
	public:
		struct Sample
		{
			int m_testId;
			std::string m_name;
			std::string m_comment;
			Offset m_imageOffset = 0;
			ImageDecorator* m_imageDec;
			Function* m_func;
			SymbolContext m_symbolCtx;
			Decompiler::FunctionPCodeGraph* m_funcGraph = nullptr;
			std::map<int64_t, DataType::IFunctionSignature*> m_functions;
			DecTestSamplesPool* m_pool;

			void decode();

			Decompiler::DecompiledCodeGraph* decompile();

			Decompiler::SdaCodeGraph* symbolize(Decompiler::DecompiledCodeGraph* decCodeGraph) const;
		};

		std::list<Sample*> m_samples;

		DecTestSamplesPool(Project* project);

		void fillByTests();

		Sample* createSampleTest(int testId, const std::string& name, const std::string& comment, AbstractImage* image,
		                         int offset = 0);

		Sample* createSampleTest(int testId, const std::string& name, const std::string& comment, std::vector<uint8_t> content);

	private:
		DataTypePtr findType(std::string typeName, std::string typeLevel = "") const;

		// calculates the function size, seeing RET instruction
		static int CalculateFuncSize(byte* addr, bool endByRet = false);

		// returns array of bytes on specified address of some function (seeing RET instruction)
		static std::vector<byte> GetFuncBytes(void* addr);
	};
};