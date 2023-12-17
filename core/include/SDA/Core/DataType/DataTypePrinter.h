#pragma once
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/DataType/DataTypeParser.h"
#include "SDA/Core/SymbolTable/SymbolTablePrinter.h"

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
        
        static std::string Print(const std::list<ParsedDataType>& parsedDataTypes, Context* context, bool withName = true, bool withId = true);

        void printDef(DataType* dataType, bool withName = true, bool withId = true, bool withBody = true);

    protected:
        void printTypeDef(TypedefDataType* typedefDt);

        void printEnumDef(EnumDataType* enumDt);

        void printStructureDef(StructureDataType* structDt, bool withBody);

        void printSignatureDef(SignatureDataType* signatureDt);

        virtual void printDataType(DataType* dataType);

        void printTokenImpl(const std::string& text, Token token) const override;
    };
};