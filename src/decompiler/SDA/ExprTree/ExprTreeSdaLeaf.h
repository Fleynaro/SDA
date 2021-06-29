#pragma once
#include "ExprTreeSdaNode.h"
#include <Code/Symbol/Symbol.h>

namespace CE::Decompiler::ExprTree
{
	// symbol that DONT related to memory: localVar1, param1, ...
	class SdaSymbolLeaf : public SdaNode, public ILeaf
	{
	public:
		SdaSymbolLeaf(CE::Symbol::ISymbol* sdaSymbol, Symbol::Symbol* decSymbol)
			: m_sdaSymbol(sdaSymbol), m_decSymbol(decSymbol)
		{}

		Symbol::Symbol* getDecSymbol() {
			return m_decSymbol;
		}

		CE::Symbol::ISymbol* getSdaSymbol() {
			return m_sdaSymbol;
		}

		int getSize() override {
			return getDataType()->getSize();
		}

		HS getHash() override {
			return m_decSymbol->getHash();
		}

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override {
			return new SdaSymbolLeaf(m_sdaSymbol, m_decSymbol);
		}

		bool isFloatingPoint() override {
			return false;
		}

		DataTypePtr getSrcDataType() override {
			return m_sdaSymbol->getDataType();
		}

		void setDataType(DataTypePtr dataType) override {
			if (m_sdaSymbol->isAutoSymbol()) {
				m_sdaSymbol->setDataType(dataType);
			}
		}

		std::string printSdaDebug() override {
			return m_sdaSymbol->getName();
		}
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
		SdaMemSymbolLeaf(CE::Symbol::IMemorySymbol* sdaSymbol, Symbol::Symbol* decSymbol, int64_t offset, bool isAddrGetting = false)
			: SdaSymbolLeaf(sdaSymbol, decSymbol), m_offset(offset), m_isAddrGetting(isAddrGetting)
		{}

		CE::Symbol::IMemorySymbol* getSdaSymbol() {
			return dynamic_cast<CE::Symbol::IMemorySymbol*>(m_sdaSymbol);
		}

		DataTypePtr getSrcDataType() override {
			if (m_isAddrGetting) {
				return MakePointer(SdaSymbolLeaf::getSrcDataType());
			}
			return SdaSymbolLeaf::getSrcDataType();
		}

		HS getHash() override {
			return SdaSymbolLeaf::getHash() << m_offset;
		}

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override {
			return new SdaMemSymbolLeaf(getSdaSymbol(), m_decSymbol, m_offset, m_isAddrGetting);
		}

		bool isAddrGetting() override {
			return m_isAddrGetting;
		}

		void setAddrGetting(bool toggle) override {
			m_isAddrGetting = toggle;
		}

		void getLocation(MemLocation& location) override {
			location.m_type = (getSdaSymbol()->getType() == CE::Symbol::LOCAL_STACK_VAR ? MemLocation::STACK : MemLocation::GLOBAL);
			location.m_offset = m_offset;
			location.m_valueSize = m_sdaSymbol->getDataType()->getSize();
		}
	};

	// 0x1000, -12, ...
	class SdaNumberLeaf : public SdaNode, public INumberLeaf
	{
		DataTypePtr m_calcDataType;
	public:
		uint64_t m_value;

		SdaNumberLeaf(uint64_t value, DataTypePtr calcDataType = nullptr)
			: m_value(value), m_calcDataType(calcDataType)
		{}

		uint64_t getValue() override {
			return m_value;
		}

		void setValue(uint64_t value) override {
			m_value = value;
		}

		int getSize() override {
			return getDataType()->getSize();
		}

		DataTypePtr getSrcDataType() override {
			return m_calcDataType;
		}

		void setDataType(DataTypePtr dataType) override {
			m_calcDataType = dataType;
		}

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override {
			return new SdaNumberLeaf(m_value, m_calcDataType);
		}

		std::string printSdaDebug() override {
			if (getSrcDataType()->isFloatingPoint()) {
				if(getSrcDataType()->getSize() == 4)
					return m_updateDebugInfo = std::to_string((float&)m_value);
				else return m_updateDebugInfo = std::to_string((double&)m_value);
			}
			if (auto sysType = dynamic_cast<DataType::SystemType*>(getSrcDataType()->getBaseType())) {
				if (sysType->isSigned()) {
					auto size = getSrcDataType()->getSize();
					if (size <= 4)
						return m_updateDebugInfo = std::to_string((int32_t)m_value);
					else
						return m_updateDebugInfo = std::to_string((int64_t)m_value);
				}
			}
			return "0x" + Helper::String::NumberToHex(m_value);
		}
	};
};