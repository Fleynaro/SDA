#pragma once
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/Utils/AbstractParser.h"

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

    class DataTypeParser : public utils::AbstractParser
    {
        friend class SymbolTableParser;
        Context* m_context;
    public:
        DataTypeParser(utils::lexer::Lexer* lexer, Context* context);

        static DataType* Parse(const std::string& text, Context* context, bool withName = true);

        DataType* parseDef(bool withName = true);

    private:
        TypedefDataType* parseTypeDef();

        EnumDataType* parseEnumDef();

        StructureDataType* parseStructureDef();

        SignatureDataType* parseSignatureDef();

        DataType* parseDataType();
    };
};