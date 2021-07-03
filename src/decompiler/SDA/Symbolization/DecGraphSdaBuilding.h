#pragma once
#include <decompiler/SDA/SdaGraphModification.h>
#include <Function.h>
#include <managers/SymbolManager.h>
#include <managers/TypeManager.h>

namespace CE::Decompiler::Symbolization
{
	// Transformation from untyped raw graph to typed one (creating sda nodes)
	class SdaBuilding : public SdaGraphModification
	{
		SymbolContext* m_symbolCtx;
		Project* m_project;
		DataType::IFunctionSignature::CallingConvetion m_callingConvention;
		std::map<Symbol::Symbol*, SdaSymbolLeaf*> m_replacedSymbols; //for cache purposes
		std::map<int64_t, CE::Symbol::ISymbol*> m_stackToSymbols; //stackVar1
		std::map<int64_t, CE::Symbol::ISymbol*> m_globalToSymbols; //globalVar1
		std::set<CE::Symbol::ISymbol*> m_newAutoSymbols; // auto-created symbols which are not defined by user (e.g. funcVar1)
		std::set<CE::Symbol::ISymbol*> m_userDefinedSymbols; // defined by user (e.g. playerObj)
		SymbolManager::Factory m_symbolFactory;
	public:

		SdaBuilding(SdaCodeGraph* sdaCodeGraph, SymbolContext* symbolCtx, Project* project, DataType::IFunctionSignature::CallingConvetion callingConvention = DataType::IFunctionSignature::FASTCALL);

		void start() override;

		std::set<CE::Symbol::ISymbol*>& getNewAutoSymbols();

		std::set<CE::Symbol::ISymbol*>& getUserDefinedSymbols();
	private:

		// join auto symbols and user symbols together
		void addSdaSymbols();

		// build high-level sda analog of low-level function node
		SdaFunctionNode* buildSdaFunctionNode(FunctionCall* funcCall);

		// build high-level sda analog of low-level number leaf
		SdaNumberLeaf* buildSdaNumberLeaf(NumberLeaf* numberLeaf) const;

		// build high-level sda analog of low-level read value node
		SdaReadValueNode* buildReadValueNode(ReadValueNode* readValueNode) const;

		// replace {node} and its childs with high-level sda analog
		void buildSdaNodesAndReplace(INode* node);

		CE::Symbol::ISymbol* findOrCreateSymbol(Symbol::Symbol* symbol, int size, int64_t& offset);

		// load stack or global memory symbol by decompiler symbol (RSP/RIP) and offset
		CE::Symbol::ISymbol* loadSdaSymbolIfMem(Symbol::Symbol* symbol, int64_t& offset);

		// store stack or global memory symbol by decompiler symbol (RSP/RIP) and offset
		void storeSdaSymbolIfMem(CE::Symbol::ISymbol* sdaSymbol, Symbol::Symbol* symbol, int64_t& offset);

		int64_t toGlobalOffset(int64_t offset) const;

		int64_t toLocalOffset(int64_t offset) const;
	};
};