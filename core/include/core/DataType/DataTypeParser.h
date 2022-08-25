#pragma once
#include "Core/DataType/DataType.h"
#include "Core/Utils/AbstractParser.h"
#include "Core/Platform/CallingConvention.h"

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

        static DataType* ParseSingle(const std::string& text, Context* context);

        std::map<std::string, DataType*> parse(char endSymbol = utils::lexer::EndSymbol);

        DataType* parseDataTypeDef(bool withName = false);

    private:
        TypedefDataType* parseTypedefDataTypeDef();

        EnumDataType* parseEnumDataTypeDef();

        StructureDataType* parseStructureDataTypeDef();

        SignatureDataType* parseSignatureDataTypeDef();

        DataType* parseDataType();
    };
};