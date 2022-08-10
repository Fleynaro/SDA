#pragma once
#include "Core/IRcode/IRcodeOperation.h"
#include "Core/DataType/SignatureDataType.h"
#include "Core/SymbolTable/SymbolTable.h"

namespace sda::decompiler
{
    class SemanticsManager;
    class Semantics
    {
    public:
        virtual std::string getName() const = 0;

        class Propagator {
            SemanticsManager* m_semManager;
        public:
            Propagator(SemanticsManager* semManager);

            struct Context {
                SymbolTable* globalSymbolTable;
                SymbolTable* stackSymbolTable;
                SymbolTable* instructionSymbolTable;
                SignatureDataType* signatureDt;
                CallingConvention::Map storages;
            };

            virtual void init(
                const Context* ctx,
                const ircode::Operation* op) = 0;

            virtual void propagate(
                const Context* ctx,
                const ircode::Operation* op,
                const Semantics* sem,
                std::list<std::shared_ptr<ircode::Variable>>& affectedVars) = 0;

        protected:
            SemanticsManager* getManager() const;

            DataType* findDataType(const std::string& name) const;
        };
    };
};