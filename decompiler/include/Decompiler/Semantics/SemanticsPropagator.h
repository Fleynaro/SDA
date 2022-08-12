#pragma once
#include "SemanticsObject.h"
#include "Semantics.h"
#include "Core/IRcode/IRcodeOperation.h"
#include "Core/Symbol/FunctionSymbol.h"

namespace sda::decompiler
{
    struct SemanticsContext {
        SymbolTable* globalSymbolTable;
        FunctionSymbol* functionSymbol;
    };

    class SemanticsManager;

    class SemanticsPropagator
    {
        SemanticsManager* m_semManager;
    public:
        SemanticsPropagator(SemanticsManager* semManager);

        virtual void propagate(
            const SemanticsContext* ctx,
            const ircode::Operation* op,
            std::list<SemanticsObject*>& nextObjs) = 0;
    protected:
        SemanticsManager* getManager() const;

        VariableSemObj* getOrCreateVarObject(std::shared_ptr<ircode::Variable> var);

        void bind(SemanticsObject* obj1, SemanticsObject* obj2);
    };

    class BaseSemanticsPropagator : public SemanticsPropagator
    {
    public:
        using SemanticsPropagator::SemanticsPropagator;

        void propagate(
            const SemanticsContext* ctx,
            const ircode::Operation* op,
            std::list<SemanticsObject*>& nextObjs) override;

    private:
        SymbolSemObj* getSymbolObject(const Symbol* symbol) const;

        SymbolTableSemObj* getSymbolTableObject(const SymbolTable* symbolTable) const;

        FuncReturnSemObj* getFuncReturnObject(const SignatureDataType* signatureDt) const;

        DataType* findDataType(const std::string& name) const;

        ScalarDataType* getScalarDataType(ScalarType scalarType, size_t size) const;

        DataTypeSemantics* createDataTypeSemantics(const DataType* dataType, const SymbolTable* symbolTable = nullptr) const;

        void setDataTypeFor(std::shared_ptr<ircode::Value> value, const DataType* dataType, std::list<SemanticsObject*>& nextObjs);
    };
};