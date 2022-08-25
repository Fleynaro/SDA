#include "Core/DataType/DataTypeParser.h"
#include "Core/DataType/VoidDataType.h"
#include "Core/DataType/PointerDataType.h"
#include "Core/DataType/ArrayDataType.h"
#include "Core/DataType/ScalarDataType.h"
#include "Core/DataType/TypedefDataType.h"
#include "Core/DataType/EnumDataType.h"
#include "Core/DataType/StructureDataType.h"
#include "Core/DataType/SignatureDataType.h"
#include "Core/SymbolTable/SymbolTableParser.h"

using namespace sda;
using namespace utils::lexer;

DataTypeParser::DataTypeParser(utils::lexer::Lexer* lexer, Context* context)
    : AbstractParser(lexer, 1), m_context(context)
{}

std::map<std::string, DataType*> DataTypeParser::Parse(const std::string& text, Context* context) {
    std::stringstream ss(text);
    IO io(ss, std::cout);
    Lexer lexer(&io);
    DataTypeParser parser(&lexer, context);
    parser.init();
    return parser.parse();
}

DataType* DataTypeParser::ParseSingle(const std::string& text, Context* context) {
    std::stringstream ss(text);
    IO io(ss, std::cout);
    Lexer lexer(&io);
    DataTypeParser parser(&lexer, context);
    parser.init();
    return parser.parseDataTypeDef();
}

std::map<std::string, DataType*> DataTypeParser::parse(char endSymbol) {
    std::map<std::string, DataType*> dataTypes;
    while (!getToken()->isSymbol(endSymbol)) {
        auto dataType = parseDataTypeDef(true);
        dataTypes[dataType->getName()] = dataType;
    }
    return dataTypes;
}

DataType* DataTypeParser::parseDataTypeDef(bool withName) {
    // comment
    auto comment = parseCommentIfExists();

    // name
    std::string name;
    if (withName) {
        if (!getToken()->isIdent(name))
            throw error(100, "Expected data type name");
        nextToken();

        accept('=');
    }

    // definition
    DataType* dataType = parseTypedefDataTypeDef();
    if (!dataType)
        dataType = parseEnumDataTypeDef();
    if (!dataType)
        dataType = parseStructureDataTypeDef();
    if (!dataType)
        dataType = parseSignatureDataTypeDef();
    if (!dataType)
        throw error(101, "Data type definition not recognized");
    
    if (dataType) {
        if (withName)
            dataType->setName(name);
        dataType->setComment(comment);
    }
    return dataType;
}

TypedefDataType* DataTypeParser::parseTypedefDataTypeDef() {
    if (!getToken()->isKeyword("typedef"))
        return nullptr;
    nextToken();
        
    auto refDt = parseDataType();
    return new TypedefDataType(
        m_context,
        nullptr,
        "",
        refDt);
}

EnumDataType* DataTypeParser::parseEnumDataTypeDef() {
    if (!getToken()->isKeyword("enum"))
        return nullptr;
    nextToken();

    accept('{');
    std::map<EnumDataType::Key, std::string> fields;
    size_t fieldIdx = 0;
    while (!getToken()->isSymbol('}')) {
        std::string fieldName;
        if (!getToken()->isIdent(fieldName))
            throw error(300, "Expected enum field name");
        nextToken();
        
        if (getToken()->isSymbol('=')) {
            nextToken();
            if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
                if (constToken->valueType != ConstToken::Integer)
                    throw error(301, "Expected integer value");
                fieldIdx = constToken->value.integer;
                nextToken();
            } else {
                throw error(302, "Expected field index");
            }
        }

        fields[fieldIdx] = fieldName;
        fieldIdx++;

        if (!getToken()->isSymbol(','))
            break;
        nextToken();
    }
    accept('}');
    return new EnumDataType(
        m_context,
        nullptr,
        "",
        fields);
}

StructureDataType* DataTypeParser::parseStructureDataTypeDef() {
    if (!getToken()->isKeyword("struct"))
        return nullptr;
    nextToken();

    SymbolTableParser symbolTableParser(getLexer(), m_context, true);
    symbolTableParser.init(std::move(getToken()));
    auto symbolTable = dynamic_cast<StandartSymbolTable*>(symbolTableParser.parse());
    assert(symbolTable);
    init(std::move(symbolTableParser.getToken()));
    return new StructureDataType(
        m_context,
        nullptr,
        "",
        symbolTable->getUsedSize(),
        symbolTable);
}

SignatureDataType* DataTypeParser::parseSignatureDataTypeDef() {
    if (!getToken()->isKeyword("signature"))
        return nullptr;
    nextToken();

    // calling convention
    std::shared_ptr<CallingConvention> callingConvention;
    for (auto& cc : m_context->getPlatform()->getCallingConventions()) {
        if (getToken()->isKeyword(cc->getName())) {
            nextToken();
            callingConvention = cc;
        }
    }
    if (!callingConvention)
        throw error(500, "Expected calling convention");
    
    // return type
    auto returnDt = parseDataType();

    // parameters
    accept('(');
    std::vector<FunctionParameterSymbol*> parameters;
    while (!getToken()->isSymbol(')')) {
        auto paramDt = parseDataType();
        std::string paramName;
        if (!getToken()->isIdent(paramName))
            throw error(501, "Expected parameter name");
        nextToken();
        auto paramSymbol = new FunctionParameterSymbol(
            m_context,
            nullptr,
            paramName,
            paramDt);
        parameters.push_back(paramSymbol);

        if (!getToken()->isSymbol(','))
            break;
        nextToken();
    }
    accept(')');

    return new SignatureDataType(
        m_context,
        callingConvention,
        nullptr,
        "",
        returnDt,
        parameters);
}

DataType* DataTypeParser::parseDataType() {
    // find data type with name
    std::string dataTypeName;
    if (!getToken()->isIdent(dataTypeName))
        throw error(200, "Expected data type name");
    auto dataType = m_context->getDataTypes()->getByName(dataTypeName);
    if (!dataType)
        throw error(201, "Unknown data type '" + dataTypeName + "'");
    nextToken();

    // pointer
    while (getToken()->isSymbol('*')) {
        dataType = dataType->getPointerTo();
        nextToken();
    }

    // array
    std::list<size_t> dimensions;
    while (getToken()->isSymbol('[')) {
        nextToken();
        size_t arraySize = -1;
        if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
            if (constToken->valueType != ConstToken::Integer)
                throw error(202, "Expected integer value");
            auto value = constToken->value.integer;
            if (value == 0)
                throw error(203, "Array size cannot be zero");
            arraySize = value;
            nextToken();
        } else {
            throw error(202, "Expected constant array size");
        }
        dimensions.push_back(arraySize);
        accept(']');
    }
    if (!dimensions.empty())
        dataType = dataType->getArrayOf(dimensions);

    return dataType;
}