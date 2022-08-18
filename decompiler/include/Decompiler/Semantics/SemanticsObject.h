#pragma once
#include <string>
#include <set>
#include "Semantics.h"
#include "SemanticsContext.h"
#include "Core/DataType/SignatureDataType.h"

namespace sda::decompiler
{
    class SemanticsObject
    {
        std::set<Semantics*> m_semantics;
        std::set<Semantics*> m_emittedSemantics;
    public:
        using Id = size_t;

        virtual Id getId() const = 0;

        virtual void bindTo(SemanticsObject* obj) = 0;

        virtual void unbindFrom(SemanticsObject* obj) = 0;

        bool addSemantics(Semantics* sem, bool emit = false);

        bool checkSemantics(const Semantics::FilterFunction& filter, bool onlyEmitted = false) const;

        std::list<Semantics*> findSemantics(const Semantics::FilterFunction& filter) const;
    };

    class VariableSemObj : public SemanticsObject
    {
        const ircode::Variable* m_variable;
        std::set<SemanticsObject*> m_relatedObjects;
        std::shared_ptr<SemanticsContext> m_context;
    public:
        VariableSemObj(const ircode::Variable* variable, const std::shared_ptr<SemanticsContext>& context);

        Id getId() const override;

        void bindTo(SemanticsObject* obj) override;

        void unbindFrom(SemanticsObject* obj) override;

        SemanticsContextOperations getRelatedOperations() const;

        const ircode::Variable* getVariable() const;

        static Id GetId(const ircode::Variable* var);
    };

    class SymbolSemObj : public SemanticsObject
    {
        const Symbol* m_symbol;
        std::set<VariableSemObj*> m_relatedVarObjects;
    public:
        SymbolSemObj(const Symbol* symbol);

        Id getId() const override;

        void bindTo(SemanticsObject* obj) override;

        void unbindFrom(SemanticsObject* obj) override;

        SemanticsContextOperations getRelatedOperations() const;

        static Id GetId(const Symbol* symbol);
    };

    class SymbolTableSemObj : public SemanticsObject
    {
        const SymbolTable* m_symbolTable;
        std::map<Offset, std::set<VariableSemObj*>> m_offsetToRelatedVarObjects;
    public:
        SymbolTableSemObj(const SymbolTable* symbolTable);

        Id getId() const override;

        void bindTo(SemanticsObject* obj) override;

        void unbindFrom(SemanticsObject* obj) override;

        SemanticsContextOperations getRelatedOperations(Offset offset) const;

        static Id GetId(const SymbolTable* symbolTable);
    };

    class FuncReturnSemObj : public SemanticsObject
    {
        const SignatureDataType* m_signatureDt;
        std::set<VariableSemObj*> m_relatedVarObjects;
    public:
        FuncReturnSemObj(const SignatureDataType* signatureDt);

        Id getId() const override;

        void bindTo(SemanticsObject* obj) override;

        void unbindFrom(SemanticsObject* obj) override;

        SemanticsContextOperations getRelatedOperations() const;

        static Id GetId(const SignatureDataType* signatureDt);
    };
};