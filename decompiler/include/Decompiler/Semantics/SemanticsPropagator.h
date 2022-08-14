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
            std::set<SemanticsObject*>& nextObjs) = 0;
    protected:
        SemanticsManager* getManager() const;

        VariableSemObj* getOrCreateVarObject(std::shared_ptr<ircode::Variable> var) const;

        void bindEachOther(SemanticsObject* obj1, SemanticsObject* obj2) const;

        void propagateTo(
            SemanticsObject* fromObj,
            SemanticsObject* toObj,
            Semantics::FilterFunction filter,
            std::set<SemanticsObject*>& nextObjs,
            size_t uncertaintyDegree = 0) const;
    };

    class BaseSemanticsPropagator : public SemanticsPropagator
    {
    public:
        using SemanticsPropagator::SemanticsPropagator;

        void propagate(
            const SemanticsContext* ctx,
            const ircode::Operation* op,
            std::set<SemanticsObject*>& nextObjs) override;

    private:
        SymbolSemObj* getSymbolObject(const Symbol* symbol) const;

        SymbolTableSemObj* getSymbolTableObject(const SymbolTable* symbolTable) const;

        FuncReturnSemObj* getFuncReturnObject(const SignatureDataType* signatureDt) const;

        DataType* findDataType(const std::string& name) const;

        ScalarDataType* getScalarDataType(ScalarType scalarType, size_t size) const;

        DataTypeSemantics* createDataTypeSemantics(
            SemanticsObject* sourceObject,
            const DataType* dataType,
            const DataTypeSemantics::SliceInfo& sliceInfo = {},
            size_t uncertaintyDegree = 0) const;

        void setDataTypeFor(std::shared_ptr<ircode::Value> value, const DataType* dataType, std::set<SemanticsObject*>& nextObjs) const;

        std::list<std::pair<Semantics*, SymbolTable*>> getAllSymbolTables(const ircode::LinearExpression& linearExpr) const;
    };
};