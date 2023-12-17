#include "SDA/Core/DataType/DataTypeParser.h"
#include "SDA/Core/DataType/VoidDataType.h"
#include "SDA/Core/DataType/PointerDataType.h"
#include "SDA/Core/DataType/ArrayDataType.h"
#include "SDA/Core/DataType/ScalarDataType.h"
#include "SDA/Core/DataType/TypedefDataType.h"
#include "SDA/Core/DataType/EnumDataType.h"
#include "SDA/Core/DataType/StructureDataType.h"
#include "SDA/Core/DataType/SignatureDataType.h"
#include "SDA/Core/SymbolTable/SymbolTableParser.h"

using namespace sda;
using namespace utils::lexer;

DataTypeParser::DataTypeParser(utils::lexer::Lexer* lexer, Context* context, ParserContext* parserContext)
    : AbstractParser(lexer, 1)
    , m_context(context)
    , m_parserContext(parserContext)
{}

std::list<ParsedDataType> DataTypeParser::Parse(const std::string& text, Context* context) {
    std::stringstream ss(text);
    IO io(ss, std::cout);
    Lexer lexer(&io, false);
    ParserContext parserContext;
    DataTypeParser parser(&lexer, context, &parserContext);
    parser.init();
    while (!parser.getToken()->isSymbol('\0')) {
        parser.parseDef();
    }
    std::list<ParsedDataType> parsedDataTypes;
    for (auto& dataTypeInfo : parserContext.dataTypes) {
        auto dataType = dataTypeInfo.create();
        dataType->setComment(dataTypeInfo.comment);
        bool isDeclared = false;
        if (dynamic_cast<StructureDataType*>(dataType)) {
            isDeclared = dataTypeInfo.size == 0;
        }
        parsedDataTypes.push_back({ dataType, isDeclared });
    }
    return parsedDataTypes;
}

const DataTypeParser::DataTypeInfo& DataTypeParser::parseDef(bool withName) {
    // comment
    auto comment = parseCommentIfExists();

    // id
    m_parserContext->currentDtId = sda::Object::Id();
    if (getToken()->isSymbol('@')) {
        nextToken();
        if (!getToken()->isKeyword("id"))
            throw error(102, "Expected @id");
        nextToken();
        if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
            if (constToken->valueType != ConstToken::String)
                throw error(103, "Expected string value");
            auto id = constToken->value.string;
            m_parserContext->currentDtId = boost::uuids::string_generator()(id);
            nextToken();
        } else {
            throw error(103, "Expected string value");
        }
    }

    // name
    std::string name;
    if (withName) {
        if (!getToken()->isIdent(name))
            throw error(100, "Expected data type name");
        nextToken();
        accept('=');
        bool isAlreadyDefined = false;
        if (auto dataType = m_context->getDataTypes()->getByName(name)) {
            isAlreadyDefined = dataType->getId() != m_parserContext->currentDtId;
        }
        if (auto dataTypeInfo = findDataTypeInfo(name)) {
            isAlreadyDefined = isAlreadyDefined || dataTypeInfo->size > 0;
        }
        if (isAlreadyDefined) {
            throw error(104, "Data type '" + name + "' already defined");
        }
    }

    m_parserContext->currentDtName = name;

    // definition
    auto dataTypeInfo = parseTypeDef();
    if (!dataTypeInfo.create)
        dataTypeInfo = parseEnumDef();
    if (!dataTypeInfo.create)
        dataTypeInfo = parseStructureDef();
    if (!dataTypeInfo.create)
        dataTypeInfo = parseSignatureDef();
    if (!dataTypeInfo.create)
        throw error(101, "Data type definition not recognized");
    
    dataTypeInfo.name = name;
    dataTypeInfo.comment = comment;
    m_parserContext->dataTypes.emplace_back(dataTypeInfo);
    return m_parserContext->dataTypes.back();
}

DataTypeParser::DataTypeInfo DataTypeParser::parseTypeDef() {
    if (!getToken()->isKeyword("typedef"))
        return DataTypeInfo();
    nextToken();
        
    auto refDtInfo = parseDataType();
    auto context = m_context;
    auto dtId = m_parserContext->currentDtId;
    auto dtName = m_parserContext->currentDtName;
    DataTypeInfo info;
    info.size = refDtInfo.size;
    info.create = [context, dtId, dtName, refDtInfo]() {
        auto refDt = refDtInfo.create();
        auto dataType = dynamic_cast<TypedefDataType*>(context->getDataTypes()->get(dtId));
        if (!dataType) {
            dataType = new TypedefDataType(context);
        }
        dataType->setName(dtName);
        dataType->setReferenceType(refDt);
        return dataType;
    };
    return info;
}

DataTypeParser::DataTypeInfo DataTypeParser::parseEnumDef() {
    if (!getToken()->isKeyword("enum"))
        return DataTypeInfo();
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
    auto context = m_context;
    auto dtId = m_parserContext->currentDtId;
    auto dtName = m_parserContext->currentDtName;
    DataTypeInfo info;
    info.size = sizeof(EnumDataType::Key);
    info.create = [context, dtId, dtName, fields]() {
        auto dataType = dynamic_cast<EnumDataType*>(context->getDataTypes()->get(dtId));
        if (!dataType) {
            dataType = new EnumDataType(context);
        }
        dataType->setName(dtName);
        dataType->setFields(fields);
        return dataType;
    };
    return info;
}

DataTypeParser::DataTypeInfo DataTypeParser::parseStructureDef() {
    if (!getToken()->isKeyword("struct"))
        return DataTypeInfo();
    nextToken();
    
    SymbolTableParser::SymbolTableInfo symbolTableInfo;
    if (getToken()->isSymbol('{')) {
        SymbolTableParser symbolTableParser(getLexer(), m_context, this, true);
        symbolTableParser.init(std::move(getToken()));
        symbolTableInfo = symbolTableParser.parse();
        init(std::move(symbolTableParser.getToken()));
    }
    auto context = m_context;
    auto dtId = m_parserContext->currentDtId;
    auto dtName = m_parserContext->currentDtName;
    DataTypeInfo info;
    info.size = symbolTableInfo.size;
    info.create = [context, dtId, dtName, symbolTableInfo]() {
        auto dataType = dynamic_cast<StructureDataType*>(dtId.is_nil() ? context->getDataTypes()->getByName(dtName) : context->getDataTypes()->get(dtId));
        if (!dataType) {
            auto symbolTable = new StandartSymbolTable(context);
            dataType = new StructureDataType(
                context,
                nullptr,
                dtName,
                0,
                symbolTable);
        }
        if (symbolTableInfo.create) {
            dataType->setName(dtName);
            dataType->setSize(symbolTableInfo.size);
            symbolTableInfo.create(dataType->getSymbolTable());
        }
        return dataType;
    };
    return info;
}

DataTypeParser::DataTypeInfo DataTypeParser::parseSignatureDef() {
    if (!getToken()->isKeyword("signature"))
        return DataTypeInfo();
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
    auto returnDtInfo = parseDataType(true);

    // parameters
    accept('(');
    struct ParamSymbol {
        std::string name;
        DataTypeInfo dataTypeInfo;
    };
    std::vector<ParamSymbol> parsedParameters;
    while (!getToken()->isSymbol(')')) {
        auto paramDtInfo = parseDataType();
        std::string paramName;
        if (!getToken()->isIdent(paramName))
            throw error(501, "Expected parameter name");
        nextToken();
        parsedParameters.push_back({ paramName, paramDtInfo });

        if (!getToken()->isSymbol(','))
            break;
        nextToken();
    }
    accept(')');

    auto context = m_context;
    auto dtId = m_parserContext->currentDtId;
    auto dtName = m_parserContext->currentDtName;
    DataTypeInfo info;
    info.size = 1;
    info.create = [context, dtId, dtName, callingConvention, parsedParameters, returnDtInfo]() {
        auto dataType = dynamic_cast<SignatureDataType*>(context->getDataTypes()->get(dtId));
        if (!dataType) {
            dataType = new SignatureDataType(context, callingConvention, nullptr, dtName);
        }
        dataType->setName(dtName);
        auto parameters = dataType->getParameters();
        for (auto i = parameters.size(); i < parsedParameters.size(); i ++) {
            parameters.push_back(new FunctionParameterSymbol(context));
        }
        for (auto i = int(parameters.size()) - 1; i >= int(parsedParameters.size()); i --) {
            parameters[i]->destroy();
            parameters.pop_back();
        }
        for (size_t i = 0; i < parsedParameters.size(); i ++) {
            auto& param = parsedParameters[i];
            parameters[i]->setName(param.name);
            parameters[i]->setDataType(param.dataTypeInfo.create());
        }
        dataType->setParameters(parameters);
        dataType->setReturnType(returnDtInfo.create());
        return dataType;
    };
    return info;
}

DataTypeParser::DataTypeInfo DataTypeParser::parseDataType(bool allowVoid) {
    // find data type with name
    std::string dataTypeName;
    if (!getToken()->isIdent(dataTypeName))
        throw error(200, "Expected data type name");
    size_t size = -1;
    if (auto dataType = m_context->getDataTypes()->getByName(dataTypeName)) {
        size = dataType->getSize();
    } else if (dataTypeName == m_parserContext->currentDtName) {
        size = 0;
    } else {
        if (auto dataTypeInfo = findDataTypeInfo(dataTypeName)) {
            size = dataTypeInfo->size;
        } else {
            throw error(201, "Unknown data type '" + dataTypeName + "'");
        }
    }
    nextToken();

    // pointer
    size_t pointerCount = 0;
    while (getToken()->isSymbol('*')) {
        pointerCount ++;
        nextToken();
    }
    if (pointerCount > 0) {
        size = m_context->getPlatform()->getPointerSize();
    }

    if (pointerCount == 0 && size == 0 && !(allowVoid && dataTypeName == "void"))
        throw error(202, "Cannot use data type '" + dataTypeName + "' without pointer");

    // array
    std::list<size_t> arrDimensions;
    while (getToken()->isSymbol('[')) {
        nextToken();
        size_t arraySize = -1;
        if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
            if (constToken->valueType != ConstToken::Integer)
                throw error(203, "Expected integer value");
            auto value = constToken->value.integer;
            if (value == 0)
                throw error(204, "Array size cannot be zero");
            arraySize = value;
            nextToken();
        } else {
            throw error(205, "Expected constant array size");
        }
        arrDimensions.push_back(arraySize);
        size *= arraySize;
        accept(']');
    }

    auto context = m_context;
    DataTypeInfo info;
    info.size = size;
    info.create = [context, dataTypeName, pointerCount, arrDimensions]() {
        auto resultDataType = context->getDataTypes()->getByName(dataTypeName);
        for (size_t i = 0; i < pointerCount; i ++) {
            resultDataType = resultDataType->getPointerTo();
        }
        if (!arrDimensions.empty())
            resultDataType = resultDataType->getArrayOf(arrDimensions);
        return resultDataType;
    };
    return info;
}

const DataTypeParser::DataTypeInfo* DataTypeParser::findDataTypeInfo(const std::string& name) {
    for (auto& dataTypeInfo : m_parserContext->dataTypes) {
        if (dataTypeInfo.name == name)
            return &dataTypeInfo;
    }
    return nullptr;
}
