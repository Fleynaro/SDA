#include "DecGraphSdaBuilding.h"
#include "SymbolTable.h"

using namespace CE;
using namespace Decompiler;
using namespace ExprTree;
using namespace Symbolization;

SdaBuilding::SdaBuilding(SdaCodeGraph* sdaCodeGraph, SymbolContext* symbolCtx, Project* project, DataType::CallingConvetion callingConvention)
	: SdaGraphModification(sdaCodeGraph), m_symbolCtx(symbolCtx), m_project(project), m_callingConvention(callingConvention), m_symbolFactory(m_project->getSymbolManager()->getFactory(false))
{
	// if signature passed in then a few first params used
	if (m_symbolCtx->m_signature) {
		for (int i = 0; i < m_symbolCtx->m_signature->getParameters().getParamsCount(); i++) {
			m_paramsUsed.insert(i + 1);
		}
	}
}

void SdaBuilding::start() {
	passAllTopNodes([&](DecBlock::BlockTopNode* topNode) {
		auto node = topNode->getNode();
		UpdateDebugInfo(node);
		buildSdaNodesAndReplace(node);
		node = topNode->getNode();
		UpdateDebugInfo(node);
		node = nullptr;
		});

	addSdaSymbols();
}

std::set<CE::Symbol::ISymbol*>& SdaBuilding::getNewAutoSymbols()
{
	return m_newAutoSymbols;
}

std::set<CE::Symbol::ISymbol*>& SdaBuilding::getUserDefinedSymbols()
{
	return m_userDefinedSymbols;
}

// join auto symbols and user symbols together

void SdaBuilding::addSdaSymbols() {
	for (auto sdaSymbol : m_newAutoSymbols) {
		m_sdaCodeGraph->getSdaSymbols().push_back(sdaSymbol);
	}

	for (auto sdaSymbol : m_userDefinedSymbols) {
		m_sdaCodeGraph->getSdaSymbols().push_back(sdaSymbol);
	}
}

// build high-level sda analog of low-level function node

SdaFunctionNode* SdaBuilding::buildSdaFunctionNode(FunctionCall* funcCall) {
	return new SdaFunctionNode(funcCall);
}

// build high-level sda analog of low-level number leaf

SdaNumberLeaf* SdaBuilding::buildSdaNumberLeaf(NumberLeaf* numberLeaf) const
{
	const auto dataType = m_project->getTypeManager()->calcDataTypeForNumber(numberLeaf->getValue());
	const auto sdaNumberLeaf = new SdaNumberLeaf(numberLeaf->getValue(), dataType);
	return sdaNumberLeaf;
}

// build high-level sda analog of low-level read value node

SdaReadValueNode* SdaBuilding::buildReadValueNode(ReadValueNode* readValueNode) const
{
	const auto dataType = m_project->getTypeManager()->getDefaultType(readValueNode->getSize());
	return new SdaReadValueNode(readValueNode, dataType);
}

// replace {node} and its childs with high-level sda analog

void SdaBuilding::buildSdaNodesAndReplace(INode* node) {
	// first process all childs
	node->iterateChildNodes([&](INode* childNode) {
		buildSdaNodesAndReplace(childNode);
		});

	if (dynamic_cast<SdaSymbolLeaf*>(node))
		return;

	if (auto readValueNode = dynamic_cast<ReadValueNode*>(node)) {
		const auto sdaReadValueNode = buildReadValueNode(readValueNode);
		readValueNode->replaceWith(sdaReadValueNode);
		readValueNode->addParentNode(sdaReadValueNode);
		return;
	}

	if (auto funcCall = dynamic_cast<FunctionCall*>(node)) {
		const auto functionNode = buildSdaFunctionNode(funcCall);
		funcCall->replaceWith(functionNode);
		funcCall->addParentNode(functionNode);
		return;
	}

	if (auto numberLeaf = dynamic_cast<NumberLeaf*>(node)) {
		const auto sdaNumberLeaf = buildSdaNumberLeaf(numberLeaf);
		numberLeaf->replaceWith(sdaNumberLeaf);
		delete numberLeaf;
		return;
	}

	// linear expr. or some symbol leaf
	const auto symbolLeaf = dynamic_cast<SymbolLeaf*>(node);
	auto linearExpr = dynamic_cast<LinearExpr*>(node);
	if (symbolLeaf || linearExpr) {
		//find symbol and offset
		int64_t offset;
		SymbolLeaf* sdaSymbolLeafToReplace = nullptr;

		if (symbolLeaf) {
			sdaSymbolLeafToReplace = symbolLeaf; // symbol found!
			offset = 0x0;
		}
		else if (linearExpr) {
			for (auto term : linearExpr->getTerms()) { // finding symbol among terms
				if (const auto termSymbolLeaf = dynamic_cast<SymbolLeaf*>(term)) {
					if (const auto regSymbol = dynamic_cast<Symbol::RegisterVariable*>(termSymbolLeaf->m_symbol)) {
						if (regSymbol->m_register.isPointer()) {
							sdaSymbolLeafToReplace = termSymbolLeaf;
							offset = linearExpr->getConstTermValue();
							break;
						}
					}
				}
			}
		}

		// if symbol has been found
		if (sdaSymbolLeafToReplace)
		{
			bool isStackOrGlobal = false;
			//check to see if this symbol is register
			if (const auto regSymbol = dynamic_cast<Symbol::RegisterVariable*>(sdaSymbolLeafToReplace->m_symbol)) {
				//if [rip] or [rsp] register
				if (regSymbol->m_register.isPointer()) {
					isStackOrGlobal = true;
				}
			}

			//not to find repeatly
			if (!isStackOrGlobal) {
				auto it = m_replacedSymbols.find(sdaSymbolLeafToReplace->m_symbol);
				if (it != m_replacedSymbols.end()) {
					NodeCloneContext ctx;
					sdaSymbolLeafToReplace->replaceWith(it->second->clone(&ctx));
					delete sdaSymbolLeafToReplace;
					return;
				}
			}

			//default size
			auto size = sdaSymbolLeafToReplace->m_symbol->getSize();
			//calculate size
			if (isStackOrGlobal) {
				//handle later anyway
				if (dynamic_cast<LinearExpr*>(node->getParentNode()))
					return;
				//if reading presence (*(float)*{[rsp] + 0x10} -> localStackVar with size of 4 bytes, not 8)
				if (auto readValueNode = dynamic_cast<ReadValueNode*>(node->getParentNode())) {
					size = readValueNode->getSize();
				}
			}
			
			//find symbol or create it
			const auto sdaSymbol = findOrCreateSymbol(sdaSymbolLeafToReplace->m_symbol, size, offset);

			// creating sda symbol leaf
			const auto newSdaSymbolLeaf = new SdaSymbolLeaf(sdaSymbolLeafToReplace, sdaSymbol, isStackOrGlobal);
			if (!isStackOrGlobal) {
				m_replacedSymbols[sdaSymbolLeafToReplace->m_symbol] = newSdaSymbolLeaf;
			}

			//replace
			sdaSymbolLeafToReplace->replaceWith(newSdaSymbolLeaf);
			sdaSymbolLeafToReplace->addParentNode(newSdaSymbolLeaf);

			if (symbolLeaf)
				return;
			if (linearExpr) {
				//if it is not array with offset is zero
				if (offset == 0x0 && linearExpr->getTerms().size() == 1) {
					linearExpr->replaceWith(newSdaSymbolLeaf);
					delete linearExpr;
					return;
				}
				//change offset
				linearExpr->setConstTermValue(offset);
			}
		}
	}

	if (dynamic_cast<DecBlock::JumpTopNode*>(node->getParentNode()))
		return;

	//otherwise create generic sda node
	const auto sdaNode = new SdaGenericNode(node, m_project->getTypeManager()->getDefaultType(node->getSize(), false, node->isFloatingPoint()));
	node->replaceWith(sdaNode);
	node->addParentNode(sdaNode);
}

CE::Symbol::ISymbol* SdaBuilding::findOrCreateSymbol(Symbol::Symbol* symbol, int size, int64_t& offset) {
	if (const auto sdaSymbol = loadSdaSymbolIfMem(symbol, offset))
		return sdaSymbol;

	//try to find corresponding function parameter if {symbol} is register (RCX -> param1, RDX -> param2)
	if (auto regSymbol = dynamic_cast<Symbol::RegisterVariable*>(symbol)) {
		auto& reg = regSymbol->m_register;
		int paramIdx = 0;

		if (m_symbolCtx->m_signature) {
			paramIdx = m_symbolCtx->m_signature->getCallInfo().findIndex(reg, offset);
			if (paramIdx > 0) {
				auto& funcParams = m_symbolCtx->m_signature->getParameters();
				if (paramIdx <= funcParams.getParamsCount()) {
					//USER-DEFINED func. parameter
					const auto sdaSymbol = funcParams[paramIdx - 1];
					storeSdaSymbolIfMem(sdaSymbol, symbol, offset);
					m_userDefinedSymbols.insert(sdaSymbol);
					return sdaSymbol;
				}
			}
		}

		// if user parameter not found then recognize
		bool isFloating = false;
		if (paramIdx <= 0) {
			if (m_callingConvention == DataType::CallingConvetion::FASTCALL) {
				GetIndex_FASTCALL(reg, offset, paramIdx, isFloating);
			}
		}

		if (paramIdx > 0) {
			if (m_paramsUsed.find(paramIdx) == m_paramsUsed.end()) {
				//auto func. parameter
				const auto defType = m_project->getTypeManager()->getDefaultType(size, false, isFloating);
				auto funcParamSymbol = m_symbolFactory.createFuncParameterSymbol(defType, "param" + std::to_string(paramIdx));
				funcParamSymbol->m_paramIdx = paramIdx;
				funcParamSymbol->setAutoSymbol(true);
				storeSdaSymbolIfMem(funcParamSymbol, symbol, offset);
				m_newAutoSymbols.insert(funcParamSymbol);
				m_paramsUsed.insert(paramIdx);
				return funcParamSymbol;
			}
		}

		//MEMORY symbol with offset (e.g. globalVar1)
		const bool isStackPointer = (reg.getType() == Register::Type::StackPointer);
		if (isStackPointer || reg.getType() == Register::Type::InstructionPointer) {
			//try to find USER-DEFINED symbol in mem. area
			CE::Symbol::AbstractSymbolTable* symTable = m_symbolCtx->m_globalSymbolTable;
			if (isStackPointer)
				symTable = m_symbolCtx->m_stackSymbolTable;
			const auto [tableOffset, tableSymbol] = symTable->getSymbolAt(offset);
			if (tableSymbol != nullptr) {
				offset -= tableOffset;
				const auto sdaSymbol = tableSymbol;
				m_userDefinedSymbols.insert(sdaSymbol);
				return sdaSymbol;
			}

			CE::Symbol::AbstractMemorySymbol* sdaMemSymbol;
			const auto dataType = m_project->getTypeManager()->getDefaultType(size);
			if (isStackPointer) {
				const auto name = "stack_0x" + Helper::String::NumberToHex(static_cast<uint32_t>(-offset));
				sdaMemSymbol = m_symbolFactory.createLocalStackVarSymbol(offset, dataType, name);
				sdaMemSymbol->setAutoSymbol(true);
			}
			else {
				const auto name = "global_0x" + Helper::String::NumberToHex(offset);
				sdaMemSymbol = m_symbolFactory.createGlobalVarSymbol(offset, dataType, name);
				sdaMemSymbol->setAutoSymbol(true);
			}
			m_newAutoSymbols.insert(sdaMemSymbol);
			storeSdaSymbolIfMem(sdaMemSymbol, symbol, offset);
			return sdaMemSymbol;
		}

		//NOT-MEMORY symbol (unknown registers)
		const auto defType = m_project->getTypeManager()->getDefaultType(size);
		auto sdaSymbol = m_symbolFactory.createLocalInstrVarSymbol(defType, "in_" + InstructionViewGenerator::GenerateRegisterName(reg));
		sdaSymbol->setAutoSymbol(true);
		m_newAutoSymbols.insert(sdaSymbol);
		return sdaSymbol;
	}

	//try to find USER-DEFINED symbol associated with some instruction
	std::list<int64_t> instrOffsets; //instruction offsets helps to identify user-defined symbols
	if (auto symbolRelToInstr = dynamic_cast<IRelatedToInstruction*>(symbol)) {
		for (auto instr : symbolRelToInstr->getInstructionsRelatedTo()) {
			instrOffsets.push_back(instr->getOffset());
		}

		if (!instrOffsets.empty()) {
			for (auto instrOffset : instrOffsets) {
				const auto symbolPair = m_symbolCtx->m_funcBodySymbolTable->getSymbolAt(instrOffset);
				if (symbolPair.second != nullptr) {
					const auto sdaSymbol = symbolPair.second;
					m_userDefinedSymbols.insert(sdaSymbol);
					return sdaSymbol;
				}
			}
		}
	}

	//otherwise create AUTO sda not-memory symbol (e.g. funcVar1)
	if (auto symbolWithId = dynamic_cast<Symbol::AbstractVariable*>(symbol)) {
		std::string suffix;
		if (dynamic_cast<Symbol::LocalVariable*>(symbol)) {
			suffix = "local";
		}
		else if (dynamic_cast<Symbol::MemoryVariable*>(symbol)) {
			suffix = "mem";
		}
		else if (dynamic_cast<Symbol::FunctionResultVar*>(symbol)) {
			suffix = "func";
		}

		const auto defType = m_project->getTypeManager()->getDefaultType(size);
		const auto name = suffix + "Var" + Helper::String::NumberToHex(symbolWithId->getId());

		auto sdaSymbol = m_symbolFactory.createLocalInstrVarSymbol(defType, name);
		sdaSymbol->setAutoSymbol(true);
		sdaSymbol->m_instrOffsets = instrOffsets;
		m_newAutoSymbols.insert(sdaSymbol);
		return sdaSymbol;
	}
	return nullptr;
}

// load stack or global memory symbol by decompiler symbol (RSP/RIP) and offset

CE::Symbol::ISymbol* SdaBuilding::loadSdaSymbolIfMem(Symbol::Symbol* symbol, int64_t& offset) {
	if (auto regSymbol = dynamic_cast<Symbol::RegisterVariable*>(symbol)) {
		auto& reg = regSymbol->m_register;
		if (reg.getType() == Register::Type::StackPointer) {
			const auto it = m_stackToSymbols.find(offset);
			if (it != m_stackToSymbols.end()) {
				offset = 0x0;
				return it->second;
			}
		}
		else if (reg.getType() == Register::Type::InstructionPointer) {
			const auto it = m_globalToSymbols.find(offset);
			if (it != m_globalToSymbols.end()) {
				offset = 0x0;
				return it->second;
			}
		}
	}
	return nullptr;
}

// store stack or global memory symbol by decompiler symbol (RSP/RIP) and offset

void SdaBuilding::storeSdaSymbolIfMem(CE::Symbol::ISymbol* sdaSymbol, Symbol::Symbol* symbol, int64_t& offset) {
	if (auto regSymbol = dynamic_cast<Symbol::RegisterVariable*>(symbol)) {
		auto& reg = regSymbol->m_register;
		if (reg.getType() == Register::Type::StackPointer) {
			m_stackToSymbols[offset] = sdaSymbol;
			offset = 0x0;
		}
		else if (reg.getType() == Register::Type::InstructionPointer) {
			m_globalToSymbols[offset] = sdaSymbol;
			offset = 0x0;
		}
		//todo: for other...
	}
}