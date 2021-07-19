#pragma once
#include "AbstractTest.h"
#include "images/VectorBufferImage.h"
//#include <decompiler_test_lib.h>

using namespace CE::Decompiler;
using namespace CE::Symbol;
using namespace CE::DataType;

class ProgramDecSampleTestFixture : public ProgramDecFixture {
public:
	// test unit for some instruction list (asm code) presented as array of bytes
	struct SampleTest
	{
		int m_testId;
		AbstractImage* m_image;
		int m_imageOffset = 0;
		SymbolContext m_symbolCtx;
		std::map<int64_t, IFunctionSignature*> m_functions;
		bool m_enabled = true;
		bool m_symbolization = true;
		bool m_showAllCode = false;
		bool m_showSymbCode = false;
		bool m_showAsmBefore = false;
		bool m_showFinalResult = false;

		void enableAllAndShowAll() {
			m_enabled = true;
			m_symbolization = true;
			m_showAllCode = true;
			m_showSymbCode = true;
			m_showAsmBefore = true;
			m_showFinalResult = true;
		}
	};

	std::list<SampleTest*> m_sampleTests;
	std::map<int, HS::Value> m_sampleTestHashes;

	//ignore all tests except
	int m_doTestIdOnly = 0;

	ProgramDecSampleTestFixture()
	{
		createProject("test");
		initSampleTestHashes();
		initSampleTest();
	}

	void initSampleTestHashes();

	void initSampleTest();

	bool checkHash(int type, std::list<std::pair<int, HS::Value>>& sampleTestHashes, HS::Value hash, SampleTest* sampleTest);

	SampleTest* createSampleTest(int testId, std::vector<uint8_t> content) {
		return createSampleTest(testId, new VectorBufferImage(
			std::vector<int8_t>(content.begin(), content.end())));
	}

	SampleTest* createSampleTest(int testId, AbstractImage* image, int offset = 0) {
		auto test = new SampleTest;
		test->m_testId = testId;
		test->m_image = image;
		test->m_imageOffset = offset;
		test->m_symbolCtx.m_globalSymbolTable = m_symTabManger->getFactory().createSymbolTable(CE::Symbol::SymbolTable::GLOBAL_SPACE);
		test->m_symbolCtx.m_funcBodySymbolTable = m_symTabManger->getFactory().createSymbolTable(CE::Symbol::SymbolTable::GLOBAL_SPACE);
		test->m_symbolCtx.m_stackSymbolTable = m_symTabManger->getFactory().createSymbolTable(CE::Symbol::SymbolTable::STACK_SPACE);
		test->m_symbolCtx.m_signature = m_defSignature;
		m_sampleTests.push_back(test);
		return test;
	}
};