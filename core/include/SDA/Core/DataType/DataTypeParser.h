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

    struct ParsedDataType {
        DataType* dataType;
        bool isDeclared;
    };

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
            sda::Object::Id currentDtId;
            std::string currentDtName;
            std::list<DataTypeInfo> dataTypes;
        };
        
    private:
        friend class SymbolTableParser;
        Context* m_context;
        ParserContext* m_parserContext;

    public:
        DataTypeParser(utils::lexer::Lexer* lexer, Context* context, ParserContext* parserContext);

        static std::list<ParsedDataType> Parse(const std::string& text, Context* context);

        const DataTypeInfo& parseDef(bool withName = true);

    private:
        DataTypeInfo parseTypeDef();

        DataTypeInfo parseEnumDef();

        DataTypeInfo parseStructureDef();

        DataTypeInfo parseSignatureDef();

        DataTypeInfo parseDataType(bool allowVoid = false);

        const DataTypeInfo* findDataTypeInfo(const std::string& name);
    };
};