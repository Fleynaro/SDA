#pragma once
#include "symbols/LocalInstrVarSymbol.h"
#include "symbols/MemorySymbol.h"
#include <map>

namespace CE {
	class ImageDecorator;
	class SymbolTableManager;
};

namespace CE::Symbol
{
	class AbstractSymbolTable : public DB::DomainObject
	{
	public:
		enum SymbolTableType {
			GLOBAL_SPACE = 1,
			STACK_SPACE = 2
		};
		
		AbstractSymbolTable(SymbolTableManager* manager, SymbolTableType type)
			: m_manager(manager), m_type(type)
		{}

		SymbolTableManager* getManager() const {
			return m_manager;
		}

		SymbolTableType getType() const {
			return m_type;
		}

		virtual void addSymbol(AbstractSymbol* symbol, int64_t offset) {
			m_symbols.insert(std::make_pair(offset, symbol));
		}

		std::pair<int64_t, AbstractSymbol*> getSymbolAt(int64_t offset) {
			const auto it = getSymbolIterator(offset);
			if (it != m_symbols.end())
				return std::make_pair(it->first, it->second);
			return std::make_pair(0, nullptr);
		}

		std::map<int64_t, AbstractSymbol*>::iterator getSymbolIterator(int64_t offset) {
			if (!m_symbols.empty()) {
				const auto it = std::prev(m_symbols.upper_bound(offset));
				if (it != m_symbols.end()) {
					const auto& [symbolOffset, symbol] = *it;
					if (offset < symbolOffset + symbol->getSize()) {
						return it;
					}
				}
			}
			return m_symbols.end();
		}

		const std::map<int64_t, AbstractSymbol*>& getSymbols() const {
			return m_symbols;
		}
	private:
		SymbolTableType m_type;
		std::map<int64_t, AbstractSymbol*> m_symbols;
		SymbolTableManager* m_manager;
	};

	class GlobalSymbolTable : public AbstractSymbolTable
	{
	public:
		ImageDecorator* m_imageDec = nullptr;
		
		GlobalSymbolTable(SymbolTableManager* manager)
			: AbstractSymbolTable(manager, GLOBAL_SPACE)
		{}

		void addSymbol(AbstractSymbol* symbol, int64_t offset) override {
			AbstractSymbolTable::addSymbol(symbol, offset);
			if(const auto globalVar = dynamic_cast<GlobalVarSymbol*>(symbol))
				globalVar->m_globalSymbolTable = this;
			else if (const auto localInstrVarSymbol = dynamic_cast<LocalInstrVarSymbol*>(symbol))
				localInstrVarSymbol->m_funcBodySymbolTable = this;
		}

		void addSymbol(GlobalVarSymbol* globalVar) {
			addSymbol(globalVar, globalVar->getOffset());
		}
	};
	
	class StackSymbolTable : public AbstractSymbolTable
	{
	public:
		StackSymbolTable(SymbolTableManager* manager)
			: AbstractSymbolTable(manager, STACK_SPACE)
		{}

		void addSymbol(AbstractSymbol* symbol, int64_t offset) override {
			AbstractSymbolTable::addSymbol(symbol, offset);
			if (const auto localStackVarSymbol = dynamic_cast<LocalStackVarSymbol*>(symbol))
				localStackVarSymbol->m_stackSymbolTable = this;
		}
	};
};