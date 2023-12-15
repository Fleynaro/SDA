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
    public:
        struct DataTypeInfo {
            size_t size = 0;
            std::string name;
            std::string comment;
            std::function<DataType*()> create;
        };

        struct ParserContext {
            std::list<DataTypeInfo> dataTypes;
        };
    private:
        friend class SymbolTableParser;
        Context* m_context;
        ParserContext* m_parserContext;
    public:

        DataTypeParser(utils::lexer::Lexer* lexer, Context* context, ParserContext* parserContext = nullptr);

        static std::list<DataType*> Parse(const std::string& text, Context* context);

        DataTypeInfo parseDef(bool withName = true);

    private:
        DataTypeInfo parseTypeDef();

        DataTypeInfo parseEnumDef();

        DataTypeInfo parseStructureDef();

        DataTypeInfo parseSignatureDef();

        DataTypeInfo parseDataType();
    };
};