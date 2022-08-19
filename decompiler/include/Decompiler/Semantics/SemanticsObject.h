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
        friend class Semantics;
        std::list<Semantics*> m_semantics;
        std::set<SemanticsObject*> m_allRelatedObjects;
    public:
        using Id = size_t;

        virtual ~SemanticsObject();

        virtual Id getId() const = 0;

        virtual void bindTo(SemanticsObject* obj);

        virtual void unbindFrom(SemanticsObject* obj);

        virtual void getAllRelatedOperations(SemanticsContextOperations& operations) const;

        bool checkSemantics(const Semantics::FilterFunction& filter) const;

        std::list<Semantics*> findSemantics(const Semantics::FilterFunction& filter) const;
    };

    class VariableSemObj : public SemanticsObject
    {
        const ircode::Variable* m_variable;
        std::shared_ptr<SemanticsContext> m_context;
    public:
        VariableSemObj(const ircode::Variable* variable, const std::shared_ptr<SemanticsContext>& context);

        Id getId() const override;

        void getAllRelatedOperations(SemanticsContextOperations& operations) const override;

        const ircode::Variable* getVariable() const;

        static Id GetId(const ircode::Variable* var);
    };

    class SymbolSemObj : public SemanticsObject
    {
        const Symbol* m_symbol;
    public:
        SymbolSemObj(const Symbol* symbol);

        Id getId() const override;

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

        void getRelatedOperationsAtOffset(SemanticsContextOperations& operations, Offset offset) const;

        static Id GetId(const SymbolTable* symbolTable);
    };

    class FuncReturnSemObj : public SemanticsObject
    {
        const SignatureDataType* m_signatureDt;
    public:
        FuncReturnSemObj(const SignatureDataType* signatureDt);

        Id getId() const override;

        static Id GetId(const SignatureDataType* signatureDt);
    };
};