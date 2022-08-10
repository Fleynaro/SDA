#pragma once
#include <list>
#include <map>
#include "Semantics.h"
#include "Core/Symbol/Symbol.h"
#include "Core/SymbolTable/SymbolTable.h"
#include "Core/DataType/DataType.h"
#include "Core/DataType/SignatureDataType.h"

namespace sda::decompiler
{
    class SemanticsManager
    {
        class SemanticsHolder {
            std::set<const Semantics*> holdedSemantics;
            std::set<const Semantics*> emitedSemantics; // subset of holdedSemantics
        public:
            SemanticsHolder() = default;

            void add(const Semantics* sem, bool emit = false);

            bool has(const Semantics* sem, bool emit = false) const;

            const Semantics* find(const std::string& name, bool emit = false) const;
        };

        Context* m_context;
        std::list<std::unique_ptr<Semantics>> m_semanticsList;
        std::map<Symbol*, SemanticsHolder> m_symbolSemanticsHolders;
        std::map<DataType*, SemanticsHolder> m_dataTypeSemanticsHolders;
        std::map<ircode::Variable*, SemanticsHolder> m_variableSemanticsHolders;
        std::map<Symbol*, std::list<ircode::Variable*>> m_symbolsToVariables;
        std::map<SymbolTable*, std::map<Offset, std::list<ircode::Variable*>>> m_symbolOffsetsToVariables;
        std::map<SignatureDataType*, std::list<ircode::Variable*>> m_funcReturnsToVariables;
    public:
        SemanticsManager(Context* context);

        Context* getContext() const;
        
        Semantics* addSemantics(std::unique_ptr<Semantics> semantics);

        SemanticsHolder* getHolder(Symbol* symbol);

        SemanticsHolder* getHolder(DataType* dataType);

        SemanticsHolder* getHolder(std::shared_ptr<ircode::Variable> variable);

        void addSymbol(Symbol* symbol, std::shared_ptr<ircode::Variable> variable);

        void addSymbolOffset(SymbolTable* symbolTable, Offset offset, std::shared_ptr<ircode::Variable> variable);

        void addFuncReturn(SignatureDataType* signatureDt, std::shared_ptr<ircode::Variable> variable);

        std::list<ircode::Variable*>& getVariablesForSymbol(Symbol* symbol);

        std::list<ircode::Variable*>& getVariablesForSymbolOffset(SymbolTable* symbolTable, Offset offset);

        std::list<ircode::Variable*>& getVariablesForFuncReturn(SignatureDataType* signatureDt);
    };
};