#pragma once
#include <decompiler/SDA/Symbolization/DecGraphSdaBuilding.h>
#include <decompiler/SDA/Symbolization/SdaGraphDataTypeCalc.h>

namespace CE::Decompiler
{
	class ProgramGraph
	{
	public:
		struct FuncGraphInfo {
			SdaCodeGraph* m_sdaFuncGraph;
			SymbolContext m_symbolCtx;
		};

	private:
		ImagePCodeGraph* m_imageGraph;
		std::map<FunctionPCodeGraph*, FuncGraphInfo> m_sdaFuncGraphs;

	public:
		ProgramGraph(ImagePCodeGraph* imageGraph)
			: m_imageGraph(imageGraph)
		{}

		ImagePCodeGraph* getImagePCodeGraph() const
		{
			return m_imageGraph;
		}

		auto& getSdaFuncGraphs() {
			return m_sdaFuncGraphs;
		}
	};

	class ImagePCodeGraphAnalyzer
	{
		// it owns raw-structure that can be changed by another during the main pass
		class RawStructureOwner : public DataType::AbstractType
		{
		public:
			class RawStructure
			{
			public:
				struct Field {
					int64_t m_offset;
					DataTypePtr m_dataType;
					bool m_isArray = false;
					// todo: add stat info like "instr. offset to datatype"
				};

				std::map<int64_t, Field> m_fields;
				std::list<RawStructureOwner*> m_owners;

				// for hierarchy
				RawStructure* m_parentRawStructure = nullptr;
				std::list<RawStructure*> m_childRawStructures;

				RawStructure(RawStructureOwner* owner)
				{
					m_owners.push_back(owner);
				}

				void removeFromOwner(RawStructureOwner* owner) {
					m_owners.remove(owner);
					if (m_owners.empty())
						delete this;
				}

				void addParent(RawStructure* rawStructure) {
					m_parentRawStructure = rawStructure;
					rawStructure->m_childRawStructures.push_back(this);
				}

				bool isSpaceEmpty(int64_t bitOffset, int bitSize) {
					if (bitOffset < 0 || bitSize <= 0)
						return false;
					return bitSize < getNextEmptyBitsCount(bitOffset);
				}

			private:
				int getNextEmptyBitsCount(int64_t bitOffset) {
					auto it = m_fields.upper_bound(bitOffset);
					if (it != m_fields.end()) {
						return int(it->first - bitOffset);
					}
					return 0x1000;
				}
			};

			int m_id;
			RawStructure* m_rawStructure;

			RawStructureOwner(int id)
				: m_id(id), DataType::AbstractType("RawStructure")
			{
				m_rawStructure = new RawStructure(this);
			}

			// if there's an empty space or there's a field with the same size
			bool canFieldBeAddedAtOffset(int64_t bitOffset, int size) const
			{
				auto it = m_rawStructure->m_fields.find(bitOffset);
				if (it != m_rawStructure->m_fields.end())
					return it->second.m_dataType->getSize() == size;
				return m_rawStructure->isSpaceEmpty(bitOffset, size * 0x8);
			}

			// create a new branch and set it current
			void createNewBranch(int64_t bitOffset) {
				auto parentRawStructure = new RawStructure(this);
				auto newRawStructure = new RawStructure(this);

				// copy fields to other raw-structures
				for (auto& pair : m_rawStructure->m_fields) {
					auto fieldOffset = pair.first;
					auto fieldDataType = pair.second;
					if (fieldOffset < bitOffset) {
						parentRawStructure->m_fields[fieldOffset] = fieldDataType;
						m_rawStructure->m_fields.erase(fieldOffset);
					}
				}

				// add parent
				m_rawStructure->addParent(parentRawStructure);
				newRawStructure->addParent(parentRawStructure);

				// set the new branch as current
				m_rawStructure->removeFromOwner(this);
				m_rawStructure = newRawStructure;
			}

			void merge(RawStructureOwner* rawStructOwner) {
				auto rawStructure = rawStructOwner->m_rawStructure;

				// add all fields
				for (auto& pair : rawStructure->m_fields) {
					auto& field = pair.second;
					if (!canFieldBeAddedAtOffset(field.m_offset, field.m_dataType->getSize()))
						createNewBranch(field.m_offset);
					addField(field);
				}

				// delete raw-structure
				for (auto owner : rawStructure->m_owners) {
					owner->m_rawStructure = m_rawStructure;
				}
				delete rawStructure;
			}

			void addField(RawStructure::Field field) const
			{
				m_rawStructure->m_fields[field.m_offset] = field;
			}

			DB::Id getId() const override {
				return m_id;
			}

			std::string getDisplayName() override {
				return "RawStructure";
			}

			Group getGroup() override {
				return Structure;
			}

			int getSize() override {
				return 0x100;
			}

			bool isUserDefined() override {
				return false;
			}
		};

		// it owns a raw-signature that can be changed by another during the main pass (it implements the decorator pattern)
		class RawSignatureOwner : public DataType::AbstractType, public DataType::IFunctionSignature
		{
		public:
			// it have stat info about return value
			struct ReturnValueStatInfo {
				Register m_register;
				int m_score = 0;
				// for marker nodes
				int m_meetMarkesCount = 0;
				int m_totalMarkesCount = 0;
			};

			// it is an extended implemetation of a func. signature that supports merge operation
			class RawSignature : public DataType::FunctionSignature
			{
			public:
				ReturnValueStatInfo m_retValStatInfo;
				std::list<RawSignatureOwner*> m_owners;
				std::list<Function*> m_functions;
				
				RawSignature(RawSignatureOwner* owner)
					: DataType::FunctionSignature("raw-signature")
				{
					m_owners.push_back(owner);
				}
			};

			RawSignature* m_rawSignature;

			RawSignatureOwner()
				: DataType::AbstractType("RawStructure")
			{
				m_rawSignature = new RawSignature(this);
			}

			~RawSignatureOwner() {
			}

			void merge(RawSignatureOwner* rawSigOwner) const
			{
				auto rawSignature = rawSigOwner->m_rawSignature;
				m_rawSignature->m_functions.insert(m_rawSignature->m_functions.begin(), rawSignature->m_functions.begin(), rawSignature->m_functions.end());
				m_rawSignature->m_owners.insert(m_rawSignature->m_owners.begin(), rawSignature->m_owners.begin(), rawSignature->m_owners.end());
				for (auto owner : rawSignature->m_owners) {
					owner->m_rawSignature = m_rawSignature;
				}
				delete rawSignature;
			}

			DB::Id getId() override {
				return 100000;
			}

			std::string getDisplayName() override {
				return "RawSignature";
			}

			Group getGroup() override {
				return FunctionSignature;
			}

			int getSize() override {
				return 0x8;
			}

			bool isUserDefined() override {
				return false;
			}

			bool isAuto() override {
				return true;
			}

			std::string getDisplayName() override {
				return m_rawSignature->getDisplayName();
			}

			CallingConvetion getCallingConvetion() override {
				return m_rawSignature->getCallingConvetion();
			}

			std::list<std::pair<int, CE::Decompiler::Storage>>& getCustomStorages() override {
				return m_rawSignature->getCustomStorages();
			}

			std::string getSigName() override {
				return m_rawSignature->getSigName();
			}

			void setReturnType(DataTypePtr returnType) override {
				return m_rawSignature->setReturnType(returnType);
			}

			DataTypePtr getReturnType() override {
				return m_rawSignature->getReturnType();
			}

			std::vector<CE::Symbol::FuncParameterSymbol*>& getParameters() override {
				return m_rawSignature->getParameters();
			}

			void addParameter(CE::Symbol::FuncParameterSymbol* symbol) override {
				return m_rawSignature->addParameter(symbol);
			}

			void addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment = "") override {
				return m_rawSignature->addParameter(name, dataType, comment);
			}

			void removeLastParameter() override {
				return m_rawSignature->removeLastParameter();
			}

			void deleteAllParameters() override {
				return m_rawSignature->deleteAllParameters();
			}

			CE::Decompiler::FunctionCallInfo getCallInfo() override {
				return m_rawSignature->getCallInfo();
			}
		};

		class StructureFinder : public Symbolization::SdaDataTypesCalculater
		{
			ImagePCodeGraphAnalyzer* m_imagePCodeGraphAnalyzer;
		public:
			StructureFinder(SdaCodeGraph* sdaCodeGraph, ImagePCodeGraphAnalyzer* imagePCodeGraphAnalyzer)
				: Symbolization::SdaDataTypesCalculater(sdaCodeGraph, nullptr, &imagePCodeGraphAnalyzer->m_dataTypeFactory), m_imagePCodeGraphAnalyzer(imagePCodeGraphAnalyzer)
			{}

		private:
			// check if it is [rcx] * 0x4
			static bool IsArrayIndexNode(ISdaNode* sdaNode) {
				if (auto sdaGenTermNode = dynamic_cast<SdaGenericNode*>(sdaNode)) {
					if (auto opNode = dynamic_cast<OperationalNode*>(sdaGenTermNode->getNode())) {
						if (auto sdaNumberLeaf = dynamic_cast<SdaNumberLeaf*>(opNode->m_rightNode)) {
							if (opNode->m_operation == Mul) {
								return true;
							}
						}
					}
				}
				return false;
			}

			void calculateDataTypes(INode* node) override {
				Symbolization::SdaDataTypesCalculater::calculateDataTypes(node);

				// only reading from memory is a trigger to define structures
				if (auto sdaReadValueNode = dynamic_cast<SdaReadValueNode*>(node)) {
					auto addrSdaNode = sdaReadValueNode->getAddress();

					// for {[rcx] + [rdx] * 0x5 + 0x10} the {sdaPointerNode} is [rcx] with the size of 8, no [rdx] * 0x5 (ambigious in the case of [rcx] + [rdx])
					ISdaNode* sdaPointerNode = nullptr;
					if (auto sdaGenNode = dynamic_cast<SdaGenericNode*>(addrSdaNode)) {
						if (auto linearExpr = dynamic_cast<LinearExpr*>(sdaGenNode)) {
							for (auto term : linearExpr->getTerms()) {
								if (auto sdaTermNode = dynamic_cast<ISdaNode*>(term)) {
									if (sdaTermNode->getSize() == 0x8 && !IsArrayIndexNode(sdaTermNode)) {
										if (!sdaPointerNode) {
											sdaPointerNode = nullptr;
											break;
										}
										sdaPointerNode = sdaTermNode;
									}
								}
							}
						}
						
					}
					else if (auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(addrSdaNode)) {
						// for *param1
						sdaPointerNode = sdaSymbolLeaf;
					}

					// create a raw structure
					if (sdaPointerNode) {
						auto rawStructOwner = m_imagePCodeGraphAnalyzer->createRawStructureOwner();
						sdaPointerNode->setDataType(DataType::GetUnit(rawStructOwner, "[1]"));
						m_nextPassRequired = true;
					}
				}
			}

			void handleFunctionNode(SdaFunctionNode* sdaFunctionNode) override {
				Symbolization::SdaDataTypesCalculater::handleFunctionNode(sdaFunctionNode);

				if (auto dstCastNode = dynamic_cast<ISdaNode*>(sdaFunctionNode->getDestination())) {
					if (auto sdaMemSymbolLeaf = dynamic_cast<SdaMemSymbolLeaf*>(dstCastNode)) {
						if (auto funcSymbol = dynamic_cast<CE::Symbol::FunctionSymbol*>(sdaMemSymbolLeaf->getSdaSymbol())) {
							// if it is a non-virtual function call
							return;
						}
					}

					if (auto rawSigOwner = dynamic_cast<RawSignatureOwner*>(dstCastNode->getDataType()->getType())) {
						auto funcCallOffset = sdaFunctionNode->getCallInstrOffset();
						m_imagePCodeGraphAnalyzer->m_virtFuncCallOffsetToSig.insert(std::pair(funcCallOffset, rawSigOwner));
						m_imagePCodeGraphAnalyzer->m_nextPassRequired = true;
					}
				}
			}

			void handleUnknownLocation(UnknownLocation* unknownLoc) override {
				// define fields of structures using the parent node: SdaReadValueNode
				if (auto readValueNode = dynamic_cast<ReadValueNode*>(unknownLoc->getParentNode())) {
					if (auto sdaReadValueNode = dynamic_cast<SdaReadValueNode*>(readValueNode->getParentNode()))
					{
						auto baseSdaNode = unknownLoc->getBaseSdaNode();
						if (auto rawStructOwner = dynamic_cast<RawStructureOwner*>(baseSdaNode->getSrcDataType()->getType())) {
							auto fieldOffset = unknownLoc->getConstTermValue();
							auto newFieldDataType = sdaReadValueNode->getDataType();

							if (!rawStructOwner->canFieldBeAddedAtOffset(fieldOffset, newFieldDataType->getSize()))
								rawStructOwner->createNewBranch(fieldOffset);

							auto it = rawStructOwner->m_rawStructure->m_fields.find(fieldOffset);
							if (it == rawStructOwner->m_rawStructure->m_fields.end() || it->second.m_dataType->getPriority() < newFieldDataType->getPriority()) {
								// set data type to the field from something
								RawStructureOwner::RawStructure::Field field;
								field.m_offset = fieldOffset;
								field.m_dataType = newFieldDataType;
								field.m_isArray = unknownLoc->getArrTerms().size() > 0; // if it is an array
								rawStructOwner->addField(field);
							}
							else {
								// set data type to something from the field
								cast(sdaReadValueNode, it->second.m_dataType);
							}
						}
					}
				}
			}

			void onDataTypeCasting(DataTypePtr fromDataType, DataTypePtr toDataType) override {
				auto dataType1 = fromDataType->getType();
				auto dataType2 = toDataType->getType();

				if (auto rawSigOwner1 = dynamic_cast<RawSignatureOwner*>(dataType1)) {
					if (auto rawSigOwner2 = dynamic_cast<RawSignatureOwner*>(dataType2)) {
						rawSigOwner1->merge(rawSigOwner2);
					}
				}

				if (auto rawStructOwner1 = dynamic_cast<RawStructureOwner*>(dataType1)) {
					if (auto rawStructOwner2 = dynamic_cast<RawStructureOwner*>(dataType2)) {
						rawStructOwner1->merge(rawStructOwner2);
					}
				}
			}
		};

		// decompiler for definition of return values for each function
		class PrimaryDecompilerForReturnVal : public AbstractPrimaryDecompiler
		{
			// it appeared after a function call that have potentially a return value
			class MarkerNode : public Node
			{
			public:
				Register m_register;
				RawSignatureOwner* m_rawSigOwner;
				bool m_hasMeet = false;

				MarkerNode(Register reg, RawSignatureOwner* rawSigOwner)
					: m_register(reg), m_rawSigOwner(rawSigOwner)
				{
					rawSigOwner->m_rawSignature->m_retValStatInfo.m_totalMarkesCount++;
				}

				// if it has been meet in the decompiled code
				void meet() {
					if (!m_hasMeet) {
						m_rawSigOwner->m_rawSignature->m_retValStatInfo.m_meetMarkesCount++;
						m_hasMeet = true;
					}
				}

				HS getHash() override {
					return HS()
						<< (uint64_t)m_rawSigOwner;
				}

				int getSize() override {
					return m_register.getSize();
				}

				INode* clone(NodeCloneContext* ctx) override {
					return new MarkerNode(m_register, m_rawSigOwner);
				}
			};

			ImagePCodeGraphAnalyzer* m_imagePCodeGraphAnalyzer = nullptr;
		public:
			using AbstractPrimaryDecompiler::AbstractPrimaryDecompiler;

			void setImagePCodeGraphAnalyzer(ImagePCodeGraphAnalyzer* imagePCodeGraphAnalyzer) {
				m_imagePCodeGraphAnalyzer = imagePCodeGraphAnalyzer;
			}

		private:
			FunctionCallInfo requestFunctionCallInfo(ExecContext* ctx, PCode::Instruction* instr, int funcOffset) override {
				auto rawSigOwner = m_imagePCodeGraphAnalyzer->getRawSignatureOwner(instr, funcOffset);
				if (rawSigOwner) {
					auto& retValStatInfo = rawSigOwner->m_rawSignature->m_retValStatInfo;
					if (retValStatInfo.m_score > 0) {
						auto& reg = retValStatInfo.m_register;
						auto markerNode = new MarkerNode(reg, rawSigOwner);
						ctx->m_registerExecCtx.setRegister(reg, markerNode);
					}
				}
					
				return FunctionCallInfo({});
			}
			
			void onFinal() {
				// find marker nodes
				for (const auto decBlock : m_decompiledGraph->getDecompiledBlocks()) {
					for (auto topNode : decBlock->getAllTopNodes()) {
						findMarkerNodes(topNode->getNode());
					}
				}
			}

			void findMarkerNodes(INode* node) {
				node->iterateChildNodes([&](INode* childNode) {
					findMarkerNodes(childNode);
					});
				if(auto markerNode = dynamic_cast<MarkerNode*>(node))
					markerNode->meet();
			}
		};

		ProgramGraph* m_programGraph;
		CE::Project* m_project;
		AbstractRegisterFactory* m_registerFactory;
		Symbolization::DataTypeFactory m_dataTypeFactory;
		PCodeGraphReferenceSearch* m_graphReferenceSearch;
		CE::Symbol::SymbolTable* m_globalSymbolTable;
		std::list<RawStructureOwner*> m_rawStructOwners;
	public:
		std::map<int64_t, RawSignatureOwner*> m_funcOffsetToSig;
		std::map<int64_t, RawSignatureOwner*> m_virtFuncCallOffsetToSig;

		ImagePCodeGraphAnalyzer(ProgramGraph* programGraph, CE::Project* programModule, AbstractRegisterFactory* registerFactory, PCodeGraphReferenceSearch* graphReferenceSearch = nullptr)
			: m_programGraph(programGraph), m_project(programModule), m_registerFactory(registerFactory), m_dataTypeFactory(programModule), m_graphReferenceSearch(graphReferenceSearch)
		{
			m_globalSymbolTable = new CE::Symbol::SymbolTable(m_project->getSymTableManager(), CE::Symbol::SymbolTable::GLOBAL_SPACE, 100000);
		}

		/*
			TODO:
			1) Создание маркеров, модификация Decompiler/Interpterer, распознавание маркеров, выставление оценкци, создание варнингов (если один маркер перезаписал другой)
			2) Сделать 2 итерации по 2 прохода(1 - retValue, 2 - типы и структуры) графа программы сначала без виртуальных вызовов, потом с вирт. вызовами.
			3) Реализовать getAllFuncCalls = getNonVirtFuncCalls + getVirtFuncCalls

			TODO 08.06.21:
			1) виртуальные вызовы добавить в граф
		*/

		void start() {
			createNewFunctions();
			createVTables();

			while (m_nextPassRequired) {
				for (auto headFuncGraph : m_programGraph->getImagePCodeGraph()->getHeadFuncGraphs()) {
					std::set<FunctionPCodeGraph*> visitedGraphs;
					doPassToDefineReturnValues(headFuncGraph, visitedGraphs);
					changeFunctionSignaturesByRetValStat();
					
					visitedGraphs.clear();
					doMainPass(headFuncGraph, visitedGraphs);
				}
			}
		}

	private:
		std::list<Function*> m_newFunctions;
		bool m_nextPassRequired = false;

		// need call after pass "to define return values"
		void changeFunctionSignaturesByRetValStat() {
			for (auto& pair : m_funcOffsetToSig) {
				auto rawSigOwner = pair.second;
				auto& retValStatInfo = rawSigOwner->m_rawSignature->m_retValStatInfo;
				// need to define if the return value from a function call requested somewhere
				retValStatInfo.m_score += retValStatInfo.m_meetMarkesCount * 5 / retValStatInfo.m_totalMarkesCount;
				
				if (retValStatInfo.m_score >= 2) {
					auto retType = m_dataTypeFactory.getDefaultType(retValStatInfo.m_register.getSize());
					rawSigOwner->setReturnType(retType);
				}

				retValStatInfo.m_score = 0;
			}
		}

		// first pass to define return values
		void doPassToDefineReturnValues(FunctionPCodeGraph* funcGraph, std::set<FunctionPCodeGraph*>& visitedGraphs) {
			visitedGraphs.insert(funcGraph);
			for (auto nextFuncGraph : funcGraph->getNonVirtFuncCalls())
				if(visitedGraphs.find(nextFuncGraph) == visitedGraphs.end())
					doPassToDefineReturnValues(nextFuncGraph, visitedGraphs);

			auto funcOffset = funcGraph->getStartBlock()->getMinOffset() >> 8;
			auto it = m_funcOffsetToSig.find(funcOffset);
			if (it == m_funcOffsetToSig.end())
				throw std::logic_error("no signature");
			auto funcSigOwner = it->second;
			// we interest function signatures without return type (void). In the next pass some of signatures will acquire a return type
			if (funcSigOwner->getReturnType()->getSize() != 0)
				return;

			DecompiledCodeGraph decompiledCodeGraph(funcGraph);
			auto funcCallInfoCallback = [&](int offset, ExprTree::INode* dst) { return FunctionCallInfo({}); };
			auto decompiler = PrimaryDecompilerForReturnVal(&decompiledCodeGraph, m_registerFactory, ReturnInfo());
			decompiler.setImagePCodeGraphAnalyzer(this);
			decompiler.start();

			// gather all end blocks (where RET command) joining them into one context
			ExecContext execContext(&decompiler);
			for (auto& pair : decompiler.m_decompiledBlocks) {
				auto& decBlockInfo = pair.second;
				if (auto block = dynamic_cast<EndDecBlock*>(decBlockInfo.m_decBlock)) {
					execContext.join(decBlockInfo.m_execCtx);
				}
			}

			// iterate over all return registers within {execContext}
			auto retRegIds = { ZYDIS_REGISTER_RAX << 8, ZYDIS_REGISTER_ZMM0 << 8 };
			RawSignatureOwner::ReturnValueStatInfo resultRetValueStatInfo;
			for (auto regId : retRegIds) {
				auto& registers = execContext.m_registerExecCtx.m_registers;
				auto it = registers.find(regId);
				if (it == registers.end())
					continue;
				auto& regList = it->second;

				// select min register (AL inside EAX)
				BitMask64 minMask(8);
				RegisterExecContext::RegisterInfo* minRegInfo = nullptr;
				for (auto& regInfo : regList) {
					if (minMask.getValue() == -1 || regInfo.m_register.m_valueRangeMask < minMask) {
						minMask = regInfo.m_register.m_valueRangeMask;
						minRegInfo = &regInfo;
					}
				}

				// give scores to the register
				if (minRegInfo) {
					RawSignatureOwner::ReturnValueStatInfo retValueStatInfo;
					retValueStatInfo.m_register = minRegInfo->m_register;
					switch (minRegInfo->m_using)
					{
					case RegisterExecContext::RegisterInfo::REGISTER_NOT_USING:
						retValueStatInfo.m_score += 5;
						break;
					case RegisterExecContext::RegisterInfo::REGISTER_PARTIALLY_USING:
						retValueStatInfo.m_score += 2;
						break;
					case RegisterExecContext::RegisterInfo::REGISTER_FULLY_USING:
						retValueStatInfo.m_score += 1;
						break;
					}

					if (resultRetValueStatInfo.m_score < retValueStatInfo.m_score)
						resultRetValueStatInfo = retValueStatInfo;
				}
			}
			// set register with the biggest score
			funcSigOwner->m_rawSignature->m_retValStatInfo = resultRetValueStatInfo;
		}

		// second pass
		void doMainPass(FunctionPCodeGraph* funcGraph, std::set<FunctionPCodeGraph*>& visitedGraphs) {
			visitedGraphs.insert(funcGraph);
			for (auto nextFuncGraph : funcGraph->getNonVirtFuncCalls())
				if (visitedGraphs.find(nextFuncGraph) == visitedGraphs.end())
					doPassToDefineReturnValues(nextFuncGraph, visitedGraphs);


			auto funcCallInfoCallback = [&](PCode::Instruction* instr, int funcOffset) {
				auto rawSigOwner = getRawSignatureOwner(instr, funcOffset);
				if (rawSigOwner)
					return rawSigOwner->getCallInfo();
				return FunctionCallInfo({});
			};
			auto decompiler = CE::Decompiler::Decompiler(funcGraph, funcCallInfoCallback, ReturnInfo(), m_registerFactory);
			decompiler.start();

			auto decCodeGraph = decompiler.getDecGraph();
			auto sdaCodeGraph = new SdaCodeGraph(decCodeGraph);

			// create symbol tables for the func. graph
			SymbolContext userSymbolDef;
			userSymbolDef.m_globalSymbolTable = m_globalSymbolTable;
			userSymbolDef.m_stackSymbolTable = new CE::Symbol::SymbolTable(m_project->getSymTableManager(), CE::Symbol::SymbolTable::STACK_SPACE, 100000);
			userSymbolDef.m_funcBodySymbolTable = new CE::Symbol::SymbolTable(m_project->getSymTableManager(), CE::Symbol::SymbolTable::GLOBAL_SPACE, 100000);

			auto funcOffset = funcGraph->getStartBlock()->getMinOffset() >> 8;
			auto it = m_funcOffsetToSig.find(funcOffset);
			if (it != m_funcOffsetToSig.end())
				userSymbolDef.m_signature = it->second->m_rawSignature;
			if (!userSymbolDef.m_signature)
				throw std::logic_error("no signature");

			ProgramGraph::FuncGraphInfo funcGraphInfo;
			funcGraphInfo.m_sdaFuncGraph = sdaCodeGraph;
			funcGraphInfo.m_symbolCtx = userSymbolDef;
			m_programGraph->getSdaFuncGraphs().insert(std::pair(funcGraph, funcGraphInfo));

			Symbolization::SdaBuilding sdaBuilding(sdaCodeGraph, &userSymbolDef, &m_dataTypeFactory);
			sdaBuilding.start();

			// gather all new symbols (only after parameters of all function will be defined)
			std::map<int, CE::Symbol::FuncParameterSymbol*> funcParamSymbols;
			for (auto symbol : sdaBuilding.getNewAutoSymbols()) {
				if (auto memSymbol = dynamic_cast<CE::Symbol::AbstractMemorySymbol*>(symbol)) {
					if (symbol->getType() == CE::Symbol::GLOBAL_VAR) {
						userSymbolDef.m_globalSymbolTable->addSymbol(memSymbol, memSymbol->getOffset());
					}
					else if (symbol->getType() == CE::Symbol::LOCAL_STACK_VAR) {
						userSymbolDef.m_stackSymbolTable->addSymbol(memSymbol, memSymbol->getOffset());
					}
				}
				else {
					if (auto localInstrVarSymbol = dynamic_cast<CE::Symbol::LocalInstrVarSymbol*>(symbol)) {
						for(auto offset : localInstrVarSymbol->m_instrOffsets)
							userSymbolDef.m_funcBodySymbolTable->addSymbol(localInstrVarSymbol, offset);
					} else if (auto funcParamSymbol = dynamic_cast<CE::Symbol::FuncParameterSymbol*>(symbol)) {
						userSymbolDef.m_signature->addParameter(funcParamSymbol);
						funcParamSymbols.insert(std::pair(funcParamSymbol->getParamIdx(), funcParamSymbol));
					}
				}
				delete symbol;
			}

			// fill the signature with new params that have been found
			if (!userSymbolDef.m_signature->getParameters().empty()) {
				// no new params appeared during second and next pass
				for (int i = 1; i < 100 && funcParamSymbols.empty(); i++) {
					auto it = funcParamSymbols.find(i);
					CE::Symbol::FuncParameterSymbol* funcParamSymbol = nullptr;
					if (it != funcParamSymbols.end()) {
						funcParamSymbol = it->second;
					}
					else {
						funcParamSymbol = new CE::Symbol::FuncParameterSymbol(
							i, userSymbolDef.m_signature, m_dataTypeFactory.getDefaultType(1), "fictiveParam" + std::to_string(i));
					}
					
					userSymbolDef.m_signature->addParameter(funcParamSymbol);
				}
			}

			StructureFinder structureFinder(sdaCodeGraph, this);
			structureFinder.start();

		}

		// find func. signature for the function call (virt. or non-virt.)
		RawSignatureOwner* getRawSignatureOwner(PCode::Instruction* instr, int funcOffset) {
			if (funcOffset != 0) {
				// if it is a non-virt. call
				auto it = m_funcOffsetToSig.find(funcOffset);
				if (it != m_funcOffsetToSig.end())
					return it->second;
				return nullptr;
			}
			// if it is a virt. call
			auto it = m_virtFuncCallOffsetToSig.find(instr->getOffset());
			if (it != m_virtFuncCallOffsetToSig.end())
				return it->second;
			return nullptr;
		}

		// create new functions for all pcode func. graphs
		void createNewFunctions() {
			for (auto funcGraph : m_programGraph->getImagePCodeGraph()->getFunctionGraphList()) {
				auto rawSignature = new RawSignatureOwner();
				auto funcOffset = funcGraph->getStartBlock()->getMinOffset() >> 8;
				m_funcOffsetToSig[funcOffset] = rawSignature;
				auto funcSymbol = new CE::Symbol::FunctionSymbol(funcOffset, DataType::GetUnit(rawSignature), "func");
				m_globalSymbolTable->addSymbol(funcSymbol, funcOffset);
				auto function = new Function(funcSymbol, funcGraph);
				rawSignature->m_rawSignature->m_functions.push_back(function);
				m_newFunctions.push_back(function);
			}
		}

		// create vtables that have been found during reference search
		void createVTables() {
			for (const auto& vtable : m_graphReferenceSearch->m_vtables) {
				auto rawStructOwner = createRawStructureOwner();
				// fill the structure with virtual functions
				int offset = 0;
				for (auto funcOffset : vtable.m_funcOffsets) {
					auto it = m_funcOffsetToSig.find(funcOffset);
					if (it != m_funcOffsetToSig.end()) {
						RawStructureOwner::RawStructure::Field field;
						field.m_offset = offset;
						field.m_dataType = DataType::GetUnit(it->second->m_rawSignature, "[1]");
						rawStructOwner->addField(field);
					}
					offset += 0x8;
				}
				// add global var for vtable
				auto vtableVarSymbol = new CE::Symbol::GlobalVarSymbol(vtable.m_offset, DataType::GetUnit(rawStructOwner), "vtable");
				m_globalSymbolTable->addSymbol(vtableVarSymbol, vtable.m_offset); // todo: intersecting might be
			}
		}

		RawStructureOwner* createRawStructureOwner() {
			auto newId = (int)m_rawStructOwners.size() + 1;
			auto rawStructOwner = new RawStructureOwner(newId);
			m_rawStructOwners.push_back(rawStructOwner);
		}
	};
};