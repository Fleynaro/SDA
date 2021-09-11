#pragma once
#include "ImageDecorator.h"
#include "decompiler/Decompiler.h"
#include "decompiler/PCode/Decompiler/PrimaryDecompiler.h"
#include "decompiler/PCode/ImageAnalyzer/DecImageAnalyzer.h"
#include "managers/ImageManager.h"
#include "managers/SymbolTableManager.h"
#include <decompiler/SDA/Symbolization/DecGraphSdaBuilding.h>
#include <decompiler/SDA/Symbolization/SdaGraphDataTypeCalc.h>

namespace CE::Decompiler
{
	class ImagePCodeGraphAnalyzer
	{
		class RawStructure : public DataType::AbstractType
		{
		public:
			struct Field {
				int64_t m_offset;
				DataTypePtr m_dataType;
				bool m_isArray = false;
				// todo: add stat info like "instr. offset to datatype"
			};
			std::map<int64_t, Field> m_fields;
			
			// for hierarchy
			std::list<RawStructure*> m_parentRawStructures;
			std::list<RawStructure*> m_childRawStructures;

			RawStructure(TypeManager* typeManager)
				: AbstractType(typeManager, "RawStructure")
			{}

			void addParent(RawStructure* rawStructure) {
				m_parentRawStructures.push_back(rawStructure);
				rawStructure->m_childRawStructures.push_back(this);
			}

			bool isSpaceEmpty(int64_t bitOffset, int bitSize) const {
				if (bitOffset < 0 || bitSize <= 0)
					return false;
				return bitSize < getNextEmptyBitsCount(bitOffset);
			}

			// if there's an empty space or there's a field with the same size
			bool canFieldBeAddedAtOffset(int64_t bitOffset, int size) const {
				const auto it = m_fields.find(bitOffset);
				if (it != m_fields.end())
					return it->second.m_dataType->getSize() == size;
				return isSpaceEmpty(bitOffset, size * 0x8);
			}

			void addField(const Field& field) {
				m_fields[field.m_offset] = field;
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

		private:
			int getNextEmptyBitsCount(int64_t bitOffset) const {
				const auto it = m_fields.upper_bound(bitOffset);
				if (it != m_fields.end()) {
					return static_cast<int>(it->first - bitOffset);
				}
				return 0x1000;
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

			// it is an extended implementation of a func. signature that supports merge operation
			class RawSignature
			{
			public:
				IFunctionSignature* m_signature;
				ReturnValueStatInfo m_retValStatInfo;
				std::list<RawSignatureOwner*> m_owners;
				std::list<Function*> m_functions;
				
				RawSignature(IFunctionSignature* signature, RawSignatureOwner* owner)
					: m_signature(signature)
				{
					m_owners.push_back(owner);
				}
			};

			RawSignature* m_rawSignature;

			RawSignatureOwner(IFunctionSignature* signature)
				: AbstractType(signature->getTypeManager(), "RawSignatureOwner")
			{
				m_rawSignature = new RawSignature(signature, this);
			}

			~RawSignatureOwner() {}

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
				return 0;
			}

			bool isInDB() override {
				return false;
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

			DataType::CallingConvetion getCallingConvetion() override {
				return m_rawSignature->m_signature->getCallingConvetion();
			}

			std::list<std::pair<int, Storage>>& getCustomStorages() override {
				return m_rawSignature->m_signature->getCustomStorages();
			}

			std::string getSigName() override {
				return m_rawSignature->m_signature->getSigName();
			}

			void setReturnType(DataTypePtr returnType) override {
				return m_rawSignature->m_signature->setReturnType(returnType);
			}

			DataTypePtr getReturnType() override {
				return m_rawSignature->m_signature->getReturnType();
			}

			DataType::ParameterList& getParameters() override {
				return m_rawSignature->m_signature->getParameters();
			}

			void addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment = "") override {
				return m_rawSignature->m_signature->addParameter(name, dataType, comment);
			}

			FunctionCallInfo getCallInfo() override {
				return m_rawSignature->m_signature->getCallInfo();
			}

			void updateParameterStorages() override {
				return m_rawSignature->m_signature->updateParameterStorages();
			}

			IFunctionSignature* clone() override {
				return m_rawSignature->m_signature->clone();
			}

			void apply(IFunctionSignature* funcSignature) override {
				return m_rawSignature->m_signature->apply(funcSignature);
			}
		};

		class StructureFinder : public Symbolization::SdaDataTypesCalculater
		{
			ImagePCodeGraphAnalyzer* m_imagePCodeGraphAnalyzer;
			bool m_excludeFunctionNodes = false;
		public:
			StructureFinder(SdaCodeGraph* sdaCodeGraph, ImagePCodeGraphAnalyzer* imagePCodeGraphAnalyzer, bool excludeFunctionNodes)
				: SdaDataTypesCalculater(sdaCodeGraph, nullptr, imagePCodeGraphAnalyzer->m_project), m_imagePCodeGraphAnalyzer(imagePCodeGraphAnalyzer), m_excludeFunctionNodes(excludeFunctionNodes)
			{}

		private:
			// check if it is [rcx] * 0x4
			static bool IsArrayIndexNode(ISdaNode* sdaNode) {
				if (const auto sdaGenTermNode = dynamic_cast<SdaGenericNode*>(sdaNode)) {
					if (const auto opNode = dynamic_cast<OperationalNode*>(sdaGenTermNode->getNode())) {
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
				SdaDataTypesCalculater::calculateDataTypes(node);

				// only reading from memory is a trigger to define structures
				if (const auto sdaReadValueNode = dynamic_cast<SdaReadValueNode*>(node)) {
					const auto addrSdaNode = sdaReadValueNode->getAddress();

					// for {[rcx] + [rdx] * 0x5 + 0x10} the {sdaPointerNode} is [rcx] with the size of 8, no [rdx] * 0x5 (ambigious in the case of [rcx] + [rdx])
					ISdaNode* sdaPointerNode = nullptr;
					if (const auto sdaGenNode = dynamic_cast<SdaGenericNode*>(addrSdaNode)) {
						if (const auto linearExpr = dynamic_cast<LinearExpr*>(sdaGenNode->getNode())) {
							for (const auto term : linearExpr->getTerms()) {
								if (const auto sdaTermNode = dynamic_cast<ISdaNode*>(term)) {
									if (sdaTermNode->getSize() == 0x8 && !IsArrayIndexNode(sdaTermNode)) {
										sdaPointerNode = sdaTermNode;
										break;
									}
								}
							}
						}
						
					}
					else if (const auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(addrSdaNode)) {
						// for *param1
						sdaPointerNode = sdaSymbolLeaf;
					}

					// create a raw structure
					if (sdaPointerNode) {
						const auto rawStructure = m_imagePCodeGraphAnalyzer->createRawStructure();
						sdaPointerNode->setDataType(GetUnit(rawStructure, "[1]"));
						m_nextPassRequired = true;
					}
				}
			}

			void handleFunctionNode(SdaFunctionNode* sdaFunctionNode) override {
				if (m_excludeFunctionNodes)
					return;
				
				SdaDataTypesCalculater::handleFunctionNode(sdaFunctionNode);

				if (const auto dstCastNode = dynamic_cast<ISdaNode*>(sdaFunctionNode->getDestination())) {
					if (const auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(dstCastNode)) {
						if (dynamic_cast<CE::Symbol::FunctionSymbol*>(sdaSymbolLeaf->getSdaSymbol())) {
							// if it is a non-virtual function call
							return;
						}
					}

					if (const auto rawSigOwner = dynamic_cast<RawSignatureOwner*>(dstCastNode->getDataType()->getType())) {
						const auto funcCallOffset = sdaFunctionNode->getCallInstrOffset();
						m_imagePCodeGraphAnalyzer->m_virtFuncCallOffsetToSig[funcCallOffset] = rawSigOwner;
						m_imagePCodeGraphAnalyzer->m_nextPassRequired = true;
					}
				}
			}

			void handleUnknownLocation(UnknownLocation* unknownLoc) override {
				// define fields of structures using the parent node: SdaReadValueNode
				if (const auto readValueNode = dynamic_cast<ReadValueNode*>(unknownLoc->getParentNode())) {
					if (const auto sdaReadValueNode = dynamic_cast<SdaReadValueNode*>(readValueNode->getParentNode()))
					{
						const auto baseSdaNode = unknownLoc->getBaseSdaNode();
						// if it is raw structure
						if (const auto rawStructure = dynamic_cast<RawStructure*>(baseSdaNode->getSrcDataType()->getType())) {
							const auto fieldOffset = unknownLoc->getConstTermValue();
							const auto newFieldDataType = sdaReadValueNode->getDataType();

							if (!rawStructure->canFieldBeAddedAtOffset(fieldOffset, newFieldDataType->getSize())) {
								// warning here (dynamic_cast required, can be only resolved by user)
							}

							const auto it = rawStructure->m_fields.find(fieldOffset);
							if (it == rawStructure->m_fields.end() || it->second.m_dataType->getPriority() < newFieldDataType->getPriority()) {
								// set data type to the field from something
								RawStructure::Field field;
								field.m_offset = fieldOffset;
								field.m_dataType = newFieldDataType;
								field.m_isArray = unknownLoc->getArrTerms().size() > 0; // if it is an array
								rawStructure->addField(field);
							}
							else {
								// set data type to something from the field
								cast(sdaReadValueNode, it->second.m_dataType);
							}
						}
					}
				}
			}

			void onDataTypeCasting(DataTypePtr fromDataType, DataTypePtr& toDataType) override {
				const auto dataType1 = fromDataType->getType();
				const auto dataType2 = toDataType->getType();

				if (const auto rawSigOwner1 = dynamic_cast<RawSignatureOwner*>(dataType1)) {
					if (const auto rawSigOwner2 = dynamic_cast<RawSignatureOwner*>(dataType2)) {
						rawSigOwner1->merge(rawSigOwner2);
					}
				}

				if (const auto rawStructure1 = dynamic_cast<RawStructure*>(dataType1)) {
					if (const auto rawStructure2 = dynamic_cast<RawStructure*>(dataType2)) {
						if (m_excludeFunctionNodes) {
							const auto rawStructure = m_imagePCodeGraphAnalyzer->createRawStructure();
							rawStructure1->addParent(rawStructure);
							rawStructure2->addParent(rawStructure);
							toDataType = GetUnit(rawStructure, "[1]");
						} else {
							// for function parameters (where type of a symbol param is more abstract than type of a param node)
							rawStructure2->addParent(rawStructure1);
						}
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
			FunctionCallInfo requestFunctionCallInfo(ExecContext* ctx, Instruction* instr, int funcOffset) override {
				if (const auto rawSigOwner = m_imagePCodeGraphAnalyzer->getRawSignatureOwner(instr, funcOffset)) {
					const auto& retValStatInfo = rawSigOwner->m_rawSignature->m_retValStatInfo;
					if (retValStatInfo.m_score > 0) {
						const auto& reg = retValStatInfo.m_register;
						const auto markerNode = new MarkerNode(reg, rawSigOwner);
						ctx->m_registerExecCtx.setRegister(reg, markerNode, instr);
					}
				}
				return FunctionCallInfo({});
			}
			
			void onFinal() override {
				// find marker nodes
				for (const auto decBlock : m_decompiledGraph->getDecompiledBlocks()) {
					for (const auto topNode : decBlock->getAllTopNodes()) {
						FindMarkerNodes(topNode->getNode());
					}
				}
			}

			static void FindMarkerNodes(INode* node) {
				node->iterateChildNodes([&](INode* childNode) {
					FindMarkerNodes(childNode);
					});
				if(auto markerNode = dynamic_cast<MarkerNode*>(node))
					markerNode->meet();
			}
		};

		ImageDecorator* m_imageDec;
		Project* m_project;
		PCodeGraphReferenceSearch* m_graphReferenceSearch;
		std::list<RawStructure*> m_rawStructures;
		std::map<int64_t, RawSignatureOwner*> m_funcOffsetToSig;
		std::map<int64_t, RawSignatureOwner*> m_virtFuncCallOffsetToSig;
		CE::Symbol::GlobalSymbolTable* m_globalSymbolTable;
		CE::Symbol::GlobalSymbolTable* m_funcBodySymbolTable;
		std::list<CE::Symbol::ISymbol*> m_allSymbols;
		bool m_nextPassRequired = false;
	public:

		ImagePCodeGraphAnalyzer(ImageDecorator* imageDec, PCodeGraphReferenceSearch* graphReferenceSearch = nullptr)
			: m_imageDec(imageDec), m_project(imageDec->getImageManager()->getProject()), m_graphReferenceSearch(graphReferenceSearch)
		{
			m_globalSymbolTable = m_project->getSymTableManager()->getFactory(false).createGlobalSymbolTable();
			m_funcBodySymbolTable = m_project->getSymTableManager()->getFactory(false).createGlobalSymbolTable();
		}

		~ImagePCodeGraphAnalyzer() {
			delete m_globalSymbolTable;
			delete m_funcBodySymbolTable;
			for(const auto& [offset, rawSigOwner] : m_funcOffsetToSig) {
				delete rawSigOwner;
			}
		}
		
		void start() {
			createRawSignatures();
			createVTables();

			do {
				m_nextPassRequired = false;
				for (const auto headFuncGraph : m_imageDec->getPCodeGraph()->getHeadFuncGraphs()) {
					std::set<FunctionPCodeGraph*> visitedGraphs;
					doPassToDefineReturnValues(headFuncGraph, visitedGraphs);
					changeFunctionSignaturesByRetValStat();
					
					visitedGraphs.clear();
					doMainPass(headFuncGraph, visitedGraphs);
				}
				break;
			} while (m_nextPassRequired);
		}

		void finish(bool markAsNew) {
			for (const auto symbol : m_allSymbols) {
				const auto dataType = symbol->getDataType();
				if (const auto rawSigOwner = dynamic_cast<RawSignatureOwner*>(dataType->getType())) {
					const auto sig = rawSigOwner->m_rawSignature->m_signature;
					symbol->setDataType(GetUnit(sig, dataType->getPointerLevels()));
					if (const auto funcSymbol = dynamic_cast<CE::Symbol::FunctionSymbol*>(symbol))
						funcSymbol->setSignature(sig);
				}
			}

			// insert all new symbols into database
			if (markAsNew) {
				for (const auto symbol : m_allSymbols) {
					if (const auto symbolDb = dynamic_cast<CE::Symbol::AbstractSymbol*>(symbol)) {
						m_project->getTransaction()->markAsNew(symbolDb);
					}
				}
			}
		}

	private:
		// need call after pass "to define return values"
		void changeFunctionSignaturesByRetValStat() {
			for (const auto& [funcOffset , rawSigOwner] : m_funcOffsetToSig) {
				auto& retValStatInfo = rawSigOwner->m_rawSignature->m_retValStatInfo;

				const auto baseScore = retValStatInfo.m_score;
				// need to define if the return value from a function call requested somewhere
				if (retValStatInfo.m_totalMarkesCount != 0) {
					retValStatInfo.m_score += retValStatInfo.m_meetMarkesCount * 5 / retValStatInfo.m_totalMarkesCount;
				}
				
				// set return type if score is high enough
				if (retValStatInfo.m_score >= 2) {
					// todo: counting on fastcall
					const auto retType = m_project->getTypeManager()->getDefaultType(
						retValStatInfo.m_register.getSize(), false, retValStatInfo.m_register.isVector());
					rawSigOwner->setReturnType(retType);
				}

				// add comment
				std::string comment = rawSigOwner->m_rawSignature->m_signature->getComment();
				comment += "Return value statistic:\n" \
					"register: "+ InstructionViewGenerator::GenerateRegisterName(retValStatInfo.m_register) +"\n" \
					"total score: "+ std::to_string(retValStatInfo.m_score) +" (base score: "+ std::to_string(baseScore) +")\n" \
					"meetMarkesCount/totalMarkesCount: "+ std::to_string(retValStatInfo.m_meetMarkesCount) +"/"+ std::to_string(retValStatInfo.m_totalMarkesCount) +"\n" \
					"\n";
				rawSigOwner->m_rawSignature->m_signature->setComment(comment);

				retValStatInfo.m_score = 0;
			}
		}

		// first pass to define return values
		void doPassToDefineReturnValues(FunctionPCodeGraph* funcGraph, std::set<FunctionPCodeGraph*>& visitedGraphs) {
			visitedGraphs.insert(funcGraph);
			for (auto nextFuncGraph : funcGraph->getNonVirtFuncCalls())
				if(visitedGraphs.find(nextFuncGraph) == visitedGraphs.end())
					doPassToDefineReturnValues(nextFuncGraph, visitedGraphs);

			const auto funcOffset = funcGraph->getStartBlock()->getMinOffset().getByteOffset();
			const auto it = m_funcOffsetToSig.find(funcOffset);
			if (it == m_funcOffsetToSig.end())
				throw std::logic_error("no signature");
			auto funcSigOwner = it->second;
			// we interest function signatures without return type (void). In the next pass some of signatures will acquire a return type
			if (funcSigOwner->getReturnType()->getSize() != 0)
				return;

			DecompiledCodeGraph decompiledCodeGraph(funcGraph);
			auto decompiler = PrimaryDecompilerForReturnVal(&decompiledCodeGraph, m_imageDec->getRegisterFactory(), ReturnInfo());
			decompiler.setImagePCodeGraphAnalyzer(this);
			decompiler.start();

			// gather all end blocks (where RET command) joining them into one context
			ExecContext execContext(&decompiler);
			for (const auto& [pcodeBlock, decBlockInfo] : decompiler.m_decompiledBlocks) {
				if (dynamic_cast<EndDecBlock*>(decBlockInfo.m_decBlock)) {
					execContext.join(decBlockInfo.m_execCtx);
				}
			}

			// iterate over all return registers within {execContext}
			auto retRegIds = { ZYDIS_REGISTER_RAX << 8, ZYDIS_REGISTER_XMM0 << 8 };
			RawSignatureOwner::ReturnValueStatInfo resultRetValueStatInfo;
			for (auto regId : retRegIds) {
				const auto& registers = execContext.m_registerExecCtx.m_registers;
				const auto it = registers.find(regId);
				if (it == registers.end())
					continue;
				const auto& regList = it->second;

				// select min register (AL inside EAX)
				BitMask64 minMask(8);
				const RegisterExecContext::RegisterInfo* minRegInfo = nullptr;
				for (auto& regInfo : regList) {
					if (regInfo.m_register.m_valueRangeMask <= minMask) {
						minMask = regInfo.m_register.m_valueRangeMask;
						minRegInfo = &regInfo;
					}
				}

				// give scores to the register
				if (minRegInfo) {
					RawSignatureOwner::ReturnValueStatInfo regRetValueStatInfo;
					regRetValueStatInfo.m_register = minRegInfo->m_register;
					switch (minRegInfo->m_using)
					{
					case RegisterExecContext::RegisterInfo::REGISTER_NOT_USING:
						regRetValueStatInfo.m_score += 5;
						break;
					case RegisterExecContext::RegisterInfo::REGISTER_PARTIALLY_USING:
						regRetValueStatInfo.m_score += 2;
						break;
					case RegisterExecContext::RegisterInfo::REGISTER_FULLY_USING:
						regRetValueStatInfo.m_score += 1;
						break;
					}

					if (resultRetValueStatInfo.m_score < regRetValueStatInfo.m_score)
						resultRetValueStatInfo = regRetValueStatInfo;
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
					doMainPass(nextFuncGraph, visitedGraphs);

			// find according function
			const auto funcOffset = funcGraph->getStartBlock()->getMinOffset().getByteOffset();
			const auto function = m_imageDec->getFunctionAt(funcOffset);
			if (!function)
				return;

			// graphs
			const auto decCodeGraph = new DecompiledCodeGraph(funcGraph);
			const auto sdaCodeGraph = new SdaCodeGraph(decCodeGraph);

			// decompile and optimize
			auto funcCallInfoCallback = [&](Instruction* instr, int funcOffset) -> FunctionCallInfo {
				auto rawSigOwner = getRawSignatureOwner(instr, funcOffset);
				if (rawSigOwner)
					return rawSigOwner->getCallInfo();
				return FunctionCallInfo({});
			};
			auto primaryDecompiler = PrimaryDecompiler(decCodeGraph, m_imageDec->getRegisterFactory(),
				function->getSignature()->getCallInfo().getReturnInfo(),
				funcCallInfoCallback);
			primaryDecompiler.start();
			Optimization::ProcessDecompiledGraph(decCodeGraph, &primaryDecompiler);

			// data type building
			auto symbolContext = function->getSymbolContext();
			Symbolization::SdaBuilding sdaBuilding(sdaCodeGraph, &symbolContext, m_project);
			sdaBuilding.start();

			// data type calculating
			StructureFinder structureFinder(sdaCodeGraph, this, true);
			structureFinder.start();
			// including function params
			StructureFinder structureFinder2(sdaCodeGraph, this, false);
			//structureFinder2.start();

			// gather all new symbols (only after parameters of all function will be defined)
			for (const auto symbol : sdaBuilding.getNewAutoSymbols()) {
				// add symbols into symbol tables
				if (const auto memSymbol = dynamic_cast<CE::Symbol::AbstractMemorySymbol*>(symbol)) {
					if (symbol->getType() == CE::Symbol::GLOBAL_VAR) {
						m_imageDec->getGlobalSymbolTable()->addSymbol(memSymbol, memSymbol->getOffset());
					}
					else if (symbol->getType() == CE::Symbol::LOCAL_STACK_VAR) {
						function->getStackSymbolTable()->addSymbol(memSymbol, memSymbol->getOffset());
					}
				}
				else {
					if (const auto localInstrVarSymbol = dynamic_cast<CE::Symbol::LocalInstrVarSymbol*>(symbol)) {
						for(auto offset : localInstrVarSymbol->m_instrOffsets)
							m_imageDec->getFuncBodySymbolTable()->addSymbol(localInstrVarSymbol, offset);
					}
				}
				m_allSymbols.push_back(symbol);
			}

			// fill the signature with params that are existing or newly found
			auto& params = function->getSignature()->getParameters();
			// gather all found params
			std::list<CE::Symbol::FuncParameterSymbol*> funcParamSymbols;
			for(const auto symbol : sdaCodeGraph->getSdaSymbols()) {
				if(auto funcParamSymbol = dynamic_cast<CE::Symbol::FuncParameterSymbol*>(symbol)) {
					funcParamSymbols.push_back(funcParamSymbol);
				}
			}
			
			// sort them
			funcParamSymbols.sort([](CE::Symbol::FuncParameterSymbol* param1, CE::Symbol::FuncParameterSymbol* param2)
				{
					return param1->getParamIdx() < param2->getParamIdx();
				});
			
			// remove old params which are not in found params
			for(int i = 0; i < params.getParamsCount(); i ++) {
				const auto it = std::find(funcParamSymbols.begin(), funcParamSymbols.end(), params[i]);
				if(it == funcParamSymbols.end()) {
					m_allSymbols.remove(params[i]);
					delete params[i];
				}
			}
			params.clear();

			// add params
			auto prevParamIdx = 0;
			for(const auto paramSymbol : funcParamSymbols) {
				// create and add fake params to fill a gap (param1, _param2, param3)
				for (int paramIdx = prevParamIdx + 1; paramIdx < paramSymbol->getParamIdx(); paramIdx++) {
					const auto defType = m_project->getTypeManager()->getDefaultType(0x4);
					const auto fakeParamSymbol = m_project->getSymbolManager()->getFactory(false).createFuncParameterSymbol(defType, "_param" + std::to_string(paramIdx));
					fakeParamSymbol->m_paramIdx = paramIdx;
					fakeParamSymbol->setAutoSymbol(true);
					params.addParameter(fakeParamSymbol);
				}
				// add parameter
				params.addParameter(paramSymbol);
				prevParamIdx = paramSymbol->getParamIdx();
			}

			delete sdaCodeGraph;
			delete decCodeGraph;
		}
		
		// find func. signature for the function call (virt. or non-virt.)
		RawSignatureOwner* getRawSignatureOwner(Instruction* instr, int funcOffset) {
			if (funcOffset != 0) {
				// if it is a non-virt. call
				const auto it = m_funcOffsetToSig.find(funcOffset);
				if (it != m_funcOffsetToSig.end())
					return it->second;
				return nullptr;
			}
			// if it is a virt. call
			const auto it = m_virtFuncCallOffsetToSig.find(instr->getOffset());
			if (it != m_virtFuncCallOffsetToSig.end())
				return it->second;
			return nullptr;
		}

		// create new raw signatures for all pcode func. graphs
		void createRawSignatures() {
			for (const auto funcGraph : m_imageDec->getPCodeGraph()->getFunctionGraphList()) {
				const auto funcOffset = funcGraph.getStartBlock()->getMinOffset().getByteOffset();
				if (const auto function = m_imageDec->getFunctionAt(funcOffset)) {
					const auto rawSignature = new RawSignatureOwner(function->getSignature());
					m_funcOffsetToSig[funcOffset] = rawSignature;
					function->getFunctionSymbol()->setSignature(rawSignature);
					function->getFunctionSymbol()->setDataType(GetUnit(rawSignature));
					rawSignature->m_rawSignature->m_functions.push_back(function);
					m_allSymbols.push_back(function->getFunctionSymbol());
				}
			}
		}

		// create vtables that have been found during reference search
		void createVTables() {
			for (const auto& vtable : m_graphReferenceSearch->m_vtables) {
				const auto rawStructOwner = createRawStructure();
				// fill the structure with virtual functions
				int offset = 0;
				for (auto funcOffset : vtable.m_funcOffsets) {
					auto it = m_funcOffsetToSig.find(funcOffset);
					if (it != m_funcOffsetToSig.end()) {
						RawStructure::Field field;
						field.m_offset = offset;
						field.m_dataType = GetUnit(it->second->m_rawSignature->m_signature, "[1]");
						rawStructOwner->addField(field);
					}
					offset += 0x8;
				}
				
				// add global var for vtable
				const auto vtableGlobalVar = m_project->getSymbolManager()->getFactory(false)
					.createGlobalVarSymbol(vtable.m_offset, GetUnit(rawStructOwner), "vtable_0x" + Helper::String::NumberToHex(vtable.m_offset));
				m_globalSymbolTable->addSymbol(vtableGlobalVar, vtable.m_offset); // todo: intersecting might be
				m_allSymbols.push_back(vtableGlobalVar);
			}
		}

		RawStructure* createRawStructure() {
			const auto rawStructOwner = new RawStructure(m_project->getTypeManager());
			m_rawStructures.push_back(rawStructOwner);
			return rawStructOwner;
		}
	};
};