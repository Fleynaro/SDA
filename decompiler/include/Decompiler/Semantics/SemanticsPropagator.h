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

        bool checkSemantics(
            const SemanticsObject* obj,
            Semantics::FilterFunction filter,
            Semantics* predSem = nullptr) const;

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
            const std::shared_ptr<Semantics::SourceInfo>& sourceInfo,
            DataType* dataType,
            const DataTypeSemantics::SliceInfo& sliceInfo = {},
            const Semantics::MetaInfo& metaInfo = {}) const;

        void setDataTypeFor(std::shared_ptr<ircode::Value> value, DataType* dataType, std::set<SemanticsObject*>& nextObjs) const;

        struct PointerInfo {
            Semantics* semantics;
            SymbolTable* symbolTable = nullptr;
            DataType* dataType = nullptr;
        };
        std::list<PointerInfo> getAllPointers(const ircode::LinearExpression& linearExpr) const;

        std::list<std::pair<Offset, Symbol*>> getAllSymbolsAt(const SemanticsContext* ctx, SymbolTable* symbolTable, Offset offset, bool write = false) const;
    };
};