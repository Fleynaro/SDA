#pragma once
#include "Core/DataType/DataType.h"
#include "Core/Utils/AbstractParser.h"

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

        static std::map<std::string, DataType*> Parse(const std::string& text, Context* context);

        static DataType* ParseSingle(const std::string& text, Context* context, bool withName = false);

        std::map<std::string, DataType*> parse(char endSymbol = utils::lexer::EndSymbol);

        DataType* parseDef(bool withName = false);

    private:
        TypedefDataType* parseTypeDef();

        EnumDataType* parseEnumDef();

        StructureDataType* parseStructureDef();

        SignatureDataType* parseSignatureDef();

        DataType* parseDataType();
    };
};