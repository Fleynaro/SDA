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
        std::list<std::shared_ptr<CallingConvention>> m_callingConventions; // todo: move to platform class
    public:
        DataTypeParser(utils::lexer::Lexer* lexer, Context* context, std::list<std::shared_ptr<CallingConvention>> callingConventions = {});

        static std::map<std::string, DataType*> Parse(const std::string& text, Context* context, std::list<std::shared_ptr<CallingConvention>> callingConventions = {});

        std::map<std::string, DataType*> parse();

    private:
        DataType* parseDataTypeDef();

        TypedefDataType* parseTypedefDataTypeDef();

        EnumDataType* parseEnumDataTypeDef();

        StructureDataType* parseStructureDataTypeDef();

        SignatureDataType* parseSignatureDataTypeDef();

        DataType* parseDataType();
    };
};