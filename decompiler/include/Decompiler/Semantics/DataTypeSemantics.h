#pragma once
#include "Semantics.h"

namespace sda::decompiler
{
    class DataTypeSemantics : public Semantics
    {
        DataType* m_dataType;
        SymbolTable* m_symbolTable;
    public:
        DataTypeSemantics(DataType* dataType, SymbolTable* symbolTable = nullptr);

        std::string getName() const override;

        class Propagator : public Semantics::Propagator {
        public:
            using Semantics::Propagator::Propagator;

            void init(
                const Context* ctx,
                const ircode::Operation* op) override;

            void propagate(
                const Context* ctx,
                const ircode::Operation* op,
                const Semantics* sem,
                std::list<std::shared_ptr<ircode::Variable>>& affectedVars) override;
        };
    };
};