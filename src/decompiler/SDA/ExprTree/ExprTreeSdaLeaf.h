#pragma once
#include "ExprTreeSdaNode.h"
#include <symbols/Symbols.h>

namespace CE::Decompiler::ExprTree
{
	// symbol that DONT related to memory: localVar1, param1, ...
	class SdaSymbolLeaf : public SdaNode, public ILeaf
	{
	public:
		SdaSymbolLeaf(CE::Symbol::ISymbol* sdaSymbol, Symbol::Symbol* decSymbol);

		Symbol::Symbol* getDecSymbol();

		CE::Symbol::ISymbol* getSdaSymbol();

		int getSize() override;

		HS getHash() override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		bool isFloatingPoint() override;

		DataTypePtr getSrcDataType() override;

		void setDataType(DataTypePtr dataType) override;

		std::string printSdaDebug() override;
	protected:
		Symbol::Symbol* m_decSymbol;
		CE::Symbol::ISymbol* m_sdaSymbol;
	};

	// symbol that related to memory: stackVar1 or globalVar1
	class SdaMemSymbolLeaf : public SdaSymbolLeaf, public IMappedToMemory
	{
		bool m_isAddrGetting;
		int64_t m_offset;
	public:
		SdaMemSymbolLeaf(CE::Symbol::IMemorySymbol* sdaSymbol, Symbol::Symbol* decSymbol, int64_t offset, bool isAddrGetting = false);

		CE::Symbol::IMemorySymbol* getSdaSymbol();

		DataTypePtr getSrcDataType() override;

		HS getHash() override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		bool isAddrGetting() override;

		void setAddrGetting(bool toggle) override;

		void getLocation(MemLocation& location) override;
	};

	// 0x1000, -12, ...
	class SdaNumberLeaf : public SdaNode, public INumberLeaf
	{
		DataTypePtr m_calcDataType;
	public:
		uint64_t m_value;

		SdaNumberLeaf(uint64_t value, DataTypePtr calcDataType = nullptr);

		uint64_t getValue() override;

		void setValue(uint64_t value) override;

		int getSize() override;

		DataTypePtr getSrcDataType() override;

		void setDataType(DataTypePtr dataType) override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		std::string printSdaDebug() override;
	};
};