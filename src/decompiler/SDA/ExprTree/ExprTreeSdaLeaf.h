#pragma once
#include "ExprTreeSdaNode.h"
#include <symbols/Symbols.h>

namespace CE::Decompiler::ExprTree
{
	// symbol that DONT related to memory: localVar1, param1, ...
	class SdaSymbolLeaf : public SdaNode, public ISymbolLeaf, public INodeAgregator, public IMappedToMemory
	{
		SymbolLeaf* m_symbolLeaf;
		CE::Symbol::ISymbol* m_sdaSymbol;
		bool m_isAddrGetting;
	public:
		SdaSymbolLeaf(SymbolLeaf* m_symbolLeaf, CE::Symbol::ISymbol* sdaSymbol, bool isAddrGetting);

		void replaceNode(INode* node, INode* newNode) override;

		std::list<INode*> getNodesList() override;

		Symbol::Symbol* getDecSymbol() const;

		CE::Symbol::ISymbol* getSdaSymbol() const;

		int getSize() override;

		HS getHash() override;

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override;

		bool isFloatingPoint() override;

		DataTypePtr getSrcDataType() override;

		void setDataType(DataTypePtr dataType) override;

		bool isAddrGetting() override;

		void setAddrGetting(bool toggle) override;

		bool getLocation(MemLocation& location) override;

		StoragePath getStoragePath() override;
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

	};
};