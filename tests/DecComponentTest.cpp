#include "DecComponentTest.h"

// MEMORY LOCATION
TEST(Decompiler, Test_MemLocation)
{
	MemLocation loc1;
	MemLocation loc2;
	loc1.m_type = MemLocation::GLOBAL;
	loc2.m_type = MemLocation::GLOBAL;

	//generic offsets
	loc1.m_offset = 0x1000;
	loc1.m_valueSize = 0x4;
	loc2.m_offset = 0x1004;
	loc2.m_valueSize = 0x4;
	ASSERT_EQ(loc1.intersect(loc2), false);
	ASSERT_EQ(loc2.intersect(loc1), false);
	loc1.m_offset = 0x1001;
	ASSERT_EQ(loc1.intersect(loc2), true);
	ASSERT_EQ(loc2.intersect(loc1), true);
	loc1.m_valueSize = 0x2;
	ASSERT_EQ(loc1.intersect(loc2), false);
	ASSERT_EQ(loc2.intersect(loc1), false);
	//negative offsets
	loc1.m_offset = -160;
	loc1.m_valueSize = 0x4;
	loc2.m_offset = -156;
	loc2.m_valueSize = 0x4;
	ASSERT_EQ(loc2.intersect(loc1), false);
	ASSERT_EQ(loc1.intersect(loc2), false);
	//array
	loc1.m_offset = 0x1000;
	loc1.addArrayDim(0x4, 20);
	loc1.m_valueSize = 0x4;
	loc2.m_offset = 0x1000 + 100;
	loc2.m_valueSize = 0x4;
	ASSERT_EQ(loc2.intersect(loc1), false);
	ASSERT_EQ(loc1.intersect(loc2), false);
	loc1.addArrayDim(0x8, 20);
	ASSERT_EQ(loc2.intersect(loc1), true);
	ASSERT_EQ(loc1.intersect(loc2), true);
}

// 1) DECODERS
TEST_F(ProgramDecCompFixture, Test_Decoder)
{
	if (false) {
		auto instructions = decode({ 0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x48, 0x83, 0xF8, 0x02, 0x0F, 0x10, 0x44, 0x24, 0x20, 0x75, 0x05, 0x0F, 0x10, 0x44, 0x24, 0x10, 0x0F, 0x11, 0x44, 0x24, 0x10 });
		showInstructions(instructions);
	}
}

// 2) VIRTUAL MACHINE
TEST_F(ProgramDecCompFixture, Test_VM)
{
	auto var1 = new SymbolVarnode(4);
	std::list<Instruction*> instructions = {
		createInstr(InstructionId::INT_ADD, new ConstantVarnode(7, 4), new ConstantVarnode(5, 4), var1), // 7 + 5 = 12
		createInstr(InstructionId::INT_MULT, new ConstantVarnode(5, 4), var1, var1), // 5 * 12 = 60
		createInstr(InstructionId::CALL, var1, nullptr, new SymbolVarnode(4))
	};
	auto constValues = executeAndCalcConstValue(instructions);
	ASSERT_EQ((*constValues.begin()).second, 60);

	//showInstructions(instructions);
	//showConstValues(constValues);
}

// 3) EXPR. OPTIMIZATION
TEST_F(ProgramDecCompFixture, Test_ExprOptim)
{
	NodeCloneContext exprCloneCtx;
	auto rcx = new CE::Decompiler::Symbol::RegisterVariable(m_registerFactoryX86.createRegister(ZYDIS_REGISTER_RCX, 8));
	auto rdx = new CE::Decompiler::Symbol::RegisterVariable(m_registerFactoryX86.createRegister(ZYDIS_REGISTER_RDX, 8));
	auto expr1 = new OperationalNode(new NumberLeaf((uint64_t)2, 8), new NumberLeaf((uint64_t)0x10, 8), Mul); // 2 * 0x10 = 0x20
	auto expr2 = new OperationalNode(new SymbolLeaf(rcx), expr1, Add); // rcx + 0x20
	auto expr3 = new OperationalNode(new SymbolLeaf(rdx), expr2, Add); // rcx + 0x20 + rdx
	auto expr4 = new OperationalNode(expr3, expr1, Add); // (rcx + 0x20 + rdx) + 0x20
	auto result = new OperationalNode(expr4, new NumberLeaf((uint64_t)0xFFFF, 8), And);

	printf("before: %s\n", result->printDebug().c_str());

	auto clone1 = new TopNode(result->clone(&exprCloneCtx));
	optimize(clone1);
	printf("after: %s\n", clone1->getNode()->printDebug().c_str());

	replaceSymbolWithExpr(clone1->getNode(), rcx, new NumberLeaf((uint64_t)0x5, 8)); // rcx -> 0x5
	replaceSymbolWithExpr(clone1->getNode(), rdx, new NumberLeaf((uint64_t)0x5, 8)); // rdx -> 0x5
	auto clone2 = new TopNode(clone1->getNode()->clone(&exprCloneCtx));
	optimize(clone2);
	ASSERT_EQ(dynamic_cast<NumberLeaf*>(clone2->getNode())->getValue(), 0x40 + 0x5 + 0x5);
}

// 4) SYMBOLIZATION
TEST_F(ProgramDecCompFixture, Test_Symbolization)
{
	std::list<Instruction*> instructions;
	auto rip = new CE::Decompiler::PCode::RegisterVarnode(m_registerFactoryX86.createInstructionPointerRegister());
	auto rsp = new CE::Decompiler::PCode::RegisterVarnode(m_registerFactoryX86.createStackPointerRegister());
	SymbolVarnode* addr = new SymbolVarnode(8);
	SymbolVarnode* val4 = new SymbolVarnode(4);
	SymbolVarnode* val8 = new SymbolVarnode(8);

	auto func = m_project->getFunctionManager()->getFactory().createFunction(0x2000, m_defSignature, m_testImage, "func");
	auto symbolCtx = func->getSymbolContext();

	switch (1)
	{
	case 1: {
		/*
			struct Player {
				float[3] pos;
				int id;
			}
		*/

		SymbolVarnode* playerPos[] = { new SymbolVarnode(4), new SymbolVarnode(4), new SymbolVarnode(4) };
		SymbolVarnode* playerId = new SymbolVarnode(4);
		auto offset = 0.5f;

		{
			auto entity = m_typeManager->getFactory().createStructure("EntityTest", "");
			entity->addField(0x0, "vec", GetUnit(m_vec3D));
			entity->addField(0xC, "id", findType("uint32_t", ""));
			symbolCtx.m_globalSymbolTable->addSymbol(m_symManager->getFactory().createGlobalVarSymbol(0x100, GetUnit(entity), "entity1"), 0x100);
			symbolCtx.m_globalSymbolTable->addSymbol(m_symManager->getFactory().createGlobalVarSymbol(0x200, GetUnit(entity), "entity2"), 0x200);
		}

		instructions = {
			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x100 + 0x0, 8), addr),
			createInstr(InstructionId::STORE, addr, new ConstantVarnode(0, 4), nullptr),

			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x100 + 0x0, 8), addr),
			createInstr(InstructionId::LOAD, addr, nullptr, playerPos[0]),
			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x100 + 0x4, 8), addr),
			createInstr(InstructionId::LOAD, addr, nullptr, playerPos[1]),
			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x100 + 0x8, 8), addr),
			createInstr(InstructionId::LOAD, addr, nullptr, playerPos[2]),
			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x100 + 0xC, 8), addr),
			createInstr(InstructionId::LOAD, addr, nullptr, playerId),

			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x100 + 0x4, 8), addr),
			createInstr(InstructionId::STORE, addr, new ConstantVarnode(0, 4), nullptr),

			createInstr(InstructionId::FLOAT_ADD, playerPos[0], new ConstantVarnode((uint32_t&)offset, 4), playerPos[0]),
			createInstr(InstructionId::FLOAT_ADD, playerPos[1], new ConstantVarnode((uint32_t&)offset, 4), playerPos[1]),
			createInstr(InstructionId::FLOAT_ADD, playerPos[2], new ConstantVarnode((uint32_t&)offset, 4), playerPos[2]),

			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x1000, 8), addr),
			//createInstr(InstructionId::CALL, addr, nullptr, nullptr, 15, 1, 1),

			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x200 + 0x0, 8), addr),
			createInstr(InstructionId::STORE, addr, playerPos[0], nullptr),
			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x200 + 0x4, 8), addr),
			createInstr(InstructionId::STORE, addr, playerPos[1], nullptr),
			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x200 + 0x8, 8), addr),
			createInstr(InstructionId::STORE, addr, playerPos[2], nullptr),
			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x200 + 0xC, 8), addr),
			createInstr(InstructionId::STORE, addr, playerId, nullptr),
		};
		break;
	}

	case 2: {
		auto rcx = new CE::Decompiler::PCode::RegisterVarnode(m_registerFactoryX86.createRegister(ZYDIS_REGISTER_RCX, 0x8));
		auto rdx = new CE::Decompiler::PCode::RegisterVarnode(m_registerFactoryX86.createRegister(ZYDIS_REGISTER_RDX, 0x8));
		SymbolVarnode* gvar_val = new SymbolVarnode(4);
		SymbolVarnode* stack_val = new SymbolVarnode(4);
		SymbolVarnode* arr_val = new SymbolVarnode(4);

		// TODO: if-else

		{
			//userSymbolDef.m_funcBodySymbolTable->addSymbol(new LocalInstrVarSymbol(symbolManager(), findType("uint32_t", "[1]"), "userVar1"), 2304);
		}

		instructions = {
			// global var
			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x100, 8), addr),
			createInstr(InstructionId::LOAD, addr, nullptr, gvar_val),

			// stack var
			createInstr(InstructionId::INT_ADD, rsp, new ConstantVarnode(0x10, 8), addr),
			createInstr(InstructionId::LOAD, addr, nullptr, stack_val),

			// array
			createInstr(InstructionId::INT_ADD, rip, new ConstantVarnode(0x10, 8), addr),
			createInstr(InstructionId::INT_MULT, rdx, new ConstantVarnode(0x4, 8), val8),
			createInstr(InstructionId::INT_ADD, addr, val8, addr),
			createInstr(InstructionId::LOAD, addr, nullptr, arr_val),

			// class field 1
			createInstr(InstructionId::INT_ADD, rcx, new ConstantVarnode(0x10, 8), addr),
			createInstr(InstructionId::LOAD, addr, nullptr, addr),
			createInstr(InstructionId::INT_ADD, addr, new ConstantVarnode(0x4, 8), addr),
			createInstr(InstructionId::LOAD, addr, nullptr, val4),
			createInstr(InstructionId::INT_MULT, val4, new ConstantVarnode(0x2, 8), val4),
			createInstr(InstructionId::STORE, addr, val4, nullptr),

			// class field 2
			createInstr(InstructionId::INT_ADD, rcx, new ConstantVarnode(0x20, 8), addr),
			createInstr(InstructionId::COPY, gvar_val, nullptr, val4),
			createInstr(InstructionId::INT_ADD, val4, stack_val, val4),
			createInstr(InstructionId::STORE, addr, val4, nullptr),

			// class field 2
			createInstr(InstructionId::INT_ADD, rcx, new ConstantVarnode(0x30, 8), addr),
			createInstr(InstructionId::STORE, addr, arr_val, nullptr),
		};
		break;
	}
	}

	auto imageGraph = new ImagePCodeGraph;
	WarningContainer warningContainer;
	PCode::DecoderX86 decoder(&m_registerFactoryX86, &m_instrPool, &warningContainer);
	
	ImageAnalyzer imageAnalyzer(new SimpleBufferImage(nullptr, 0), imageGraph, &decoder, &m_registerFactoryX86);
	imageAnalyzer.start(0, true);

	auto graph = &*imageGraph->getFunctionGraphList().begin();

	auto funcCallInfoCallback = [&](PCode::Instruction* instr, int offset) { return m_defSignature->getCallInfo(); };
	auto decompiler = new CE::Decompiler::Decompiler(graph, funcCallInfoCallback, func->getSignature()->getCallInfo().getReturnInfo(), &m_registerFactoryX86);
	decompiler->start();

	auto decCodeGraph = decompiler->getDecGraph();
	showDecGraph(decCodeGraph);

	auto sdaCodeGraph = new SdaCodeGraph(decCodeGraph);
	Symbolization::SdaBuilding sdaBuilding(sdaCodeGraph, &symbolCtx, m_project);
	sdaBuilding.start();
	printf(Misc::ShowAllSymbols(sdaCodeGraph).c_str());
	showDecGraph(sdaCodeGraph->getDecGraph());

	Symbolization::SdaDataTypesCalculater sdaDataTypesCalculater(sdaCodeGraph, symbolCtx.m_signature, m_project);
	sdaDataTypesCalculater.start();
	printf(Misc::ShowAllSymbols(sdaCodeGraph).c_str());
	showDecGraph(sdaCodeGraph->getDecGraph());

	Optimization::SdaGraphMemoryOptimization memoryOptimization(sdaCodeGraph);
	memoryOptimization.start();
	showDecGraph(sdaCodeGraph->getDecGraph(), true);
}