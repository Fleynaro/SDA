#pragma once
#include "Core/DataType/DataType.h"
#include "Core/SymbolTable/SymbolTablePrinter.h"

namespace sda
{
    class VoidDataType;
    class PointerDataType;
    class ArrayDataType;
    class ScalarDataType;
    class EnumDataType;
    class TypedefDataType;
    class SignatureDataType;
    class StructureDataType;

    class DataTypePrinter : public utils::AbstractPrinter
    {
        Context* m_context;
        SymbolTablePrinter* m_symbolTablePrinter;
    public:
        static inline const Token DATATYPE = PARENT;

        DataTypePrinter(Context* context, SymbolTablePrinter* symbolTablePrinter = nullptr);
        
        static std::string Print(DataType* dataType, Context* context, bool withName = true);

        void printDef(DataType* dataType, bool withName = true);

    protected:
        void printTypeDef(TypedefDataType* typedefDt);

        void printEnumDef(EnumDataType* enumDt);

        void printStructureDef(StructureDataType* structDt);

        void printSignatureDef(SignatureDataType* signatureDt);

        virtual void printDataType(DataType* dataType);

        void printTokenImpl(const std::string& text, Token token) const override;
    };
};