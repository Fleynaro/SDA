#pragma once
#include "Core/IRcode/IRcodeOperation.h"
#include "Core/DataType/SignatureDataType.h"
#include "Core/SymbolTable/SymbolTable.h"

namespace sda::decompiler
{
    class IRcodeDataTypePropagator
    {
        Context* m_context;
        SignatureDataType* m_signatureDt;
        SymbolTable* m_globalSymbolTable;
        SymbolTable* m_stackSymbolTable;
        SymbolTable* m_instructionSymbolTable;
        CallingConvention::Map m_storages;
    public:
        IRcodeDataTypePropagator(
            Context* context,
            SignatureDataType* signatureDt,
            SymbolTable* globalSymbolTable,
            SymbolTable* stackSymbolTable,
            SymbolTable* instructionSymbolTable);
        
        void propagate(const ircode::Operation* operation);

    private:
        DataType* findDataType(const std::string& name) const;

        ScalarDataType* getScalarDataType(ScalarType scalarType, size_t size) const;

        void setDefaultDataType(std::shared_ptr<ircode::Value> value) const;
    };
};