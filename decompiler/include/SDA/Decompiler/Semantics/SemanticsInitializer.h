#pragma once
#include <list>
#include <map>
#include "SemanticsManager.h"
#include "SDA/Core/DataType/SignatureDataType.h"

namespace sda::decompiler
{
    class BaseSemanticsInitializer
    {
        SemanticsManager* m_semManager;
    public:
        BaseSemanticsInitializer(SemanticsManager* semManager);

        void addSymbolTable(SymbolTable* symbolTable);

        void addSymbol(Symbol* symbol);

        void addFuncReturn(SignatureDataType* signatureDt);

        void addDataType(DataType* dataType);

    private:
        std::shared_ptr<Semantics::SourceInfo> getSourceInfo() const;
    };
};