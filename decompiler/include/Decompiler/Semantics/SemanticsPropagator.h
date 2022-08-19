#pragma once
#include "SemanticsObject.h"
#include "Semantics.h"

namespace sda::decompiler
{
    class SemanticsManager;

    class SemanticsPropagator
    {
        SemanticsManager* m_semManager;
    public:
        SemanticsPropagator(SemanticsManager* semManager);

        virtual void propagate(
            const std::shared_ptr<SemanticsContext>& ctx,
            const ircode::Operation* op,
            SemanticsContextOperations& nextOps) = 0;
    protected:
        SemanticsManager* getManager() const;

        VariableSemObj* getOrCreateVarObject(
            const std::shared_ptr<SemanticsContext>& ctx,
            const std::shared_ptr<ircode::Variable>& var) const;

        void bindEachOther(SemanticsObject* obj1, SemanticsObject* obj2) const;

        void markAsEffected(SemanticsObject* obj, SemanticsContextOperations& nextOps) const;

        bool checkSemantics(
            const SemanticsObject* obj,
            Semantics::FilterFunction filter,
            Semantics* predSem = nullptr) const;

        void propagateTo(
            SemanticsObject* fromObj,
            SemanticsObject* toObj,
            Semantics::FilterFunction filter,
            SemanticsContextOperations& nextOps,
            size_t uncertaintyDegree = 0) const;
    };

    class BaseSemanticsPropagator : public SemanticsPropagator
    {
    public:
        using SemanticsPropagator::SemanticsPropagator;

        void propagate(
            const std::shared_ptr<SemanticsContext>& ctx,
            const ircode::Operation* op,
            SemanticsContextOperations& nextOps) override;

    private:
        SymbolSemObj* getSymbolObject(const Symbol* symbol) const;

        SymbolTableSemObj* getSymbolTableObject(const SymbolTable* symbolTable) const;

        FuncReturnSemObj* getFuncReturnObject(const SignatureDataType* signatureDt) const;

        DataType* findDataType(const std::string& name) const;

        ScalarDataType* getScalarDataType(ScalarType scalarType, size_t size) const;

        DataTypeSemantics* createDataTypeSemantics(
            SemanticsObject* holder,
            const std::shared_ptr<Semantics::SourceInfo>& sourceInfo,
            DataType* dataType,
            const DataTypeSemantics::SliceInfo& sliceInfo = {},
            const Semantics::MetaInfo& metaInfo = {}) const;

        void setDataTypeFor(
            const std::shared_ptr<SemanticsContext>& ctx,
            std::shared_ptr<ircode::Value> value,
            DataType* dataType,
            SemanticsContextOperations& nextOps) const;

        struct PointerInfo {
            Semantics* semantics;
            SymbolTable* symbolTable = nullptr;
            DataType* dataType = nullptr;
        };
        std::list<PointerInfo> getAllPointers(
            const std::shared_ptr<SemanticsContext>& ctx,
            const ircode::LinearExpression& linearExpr) const;

        std::list<std::pair<Offset, Symbol*>> getAllSymbolsAt(
            const std::shared_ptr<SemanticsContext>& ctx,
            SymbolTable* symbolTable,
            Offset offset,
            bool write = false) const;
    };
};