#include "SDA/Core/SymbolTable/SymbolTableParser.h"
#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/Symbol/VariableSymbol.h"
#include "SDA/Core/Symbol/StructureFieldSymbol.h"
#include "SDA/Core/DataType/DataTypeParser.h"

using namespace sda;
using namespace utils::lexer;

SymbolTableParser::SymbolTableParser(
    utils::lexer::Lexer* lexer,
    Context* context,
    DataTypeParser* dataTypeParser,
    bool isStruct,
    bool removeSymbols
)
    : AbstractParser(lexer, 2)
    , m_context(context)
    , m_isStruct(isStruct)
    , m_dataTypeParser(dataTypeParser)
    , m_removeSymbols(removeSymbols)
{}

SymbolTable* SymbolTableParser::Parse(
    const std::string& text,
    Context* context,
    bool isStruct,
    bool withName,
    SymbolTable* symbolTable,
    bool removeSymbols)
{
    std::stringstream ss(text);
    IO io(ss, std::cout);
    Lexer lexer(&io, false);
    DataTypeParser::ParserContext parserCtx;
    DataTypeParser dataTypeParser(&lexer, context, &parserCtx);
    SymbolTableParser parser(&lexer, context, &dataTypeParser, isStruct, removeSymbols);
    parser.init();
    auto symbolTableInfo = parser.parse(withName);
    return symbolTableInfo.create(symbolTable);
}

SymbolTableParser::SymbolTableInfo SymbolTableParser::parse(bool withName) {
    std::string comment;
    std::string name;
    if (withName && !m_isStruct) {
        comment = parseCommentIfExists();
        
        if (!getToken()->isIdent(name))
            throw error(100, "Expected symbol table name");
        nextToken();

        accept('=');
    }

    accept('{');
    std::list<SymbolInfo> symbols;
    Offset offset = 0;
    while (!getToken()->isSymbol('}')) {
        // parse symbol
        auto symbolInfo = parseSymbolDef();
        // parse offset if exists
        if (getToken()->isSymbol('=')) {
            nextToken();
            if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
                if (constToken->valueType != ConstToken::Integer)
                    throw error(101, "Expected integer value");
                auto nextOffset = constToken->value.integer;
                if (nextOffset < offset)
                    throw error(102, "Field offset is less than previous");
                offset = nextOffset;
                nextToken();
            } else {
                throw error(103, "Expected offset");
            }
        }

        symbolInfo.offset = offset;
        symbols.push_back(symbolInfo);
        offset += symbolInfo.size;

        if (!getToken()->isSymbol(','))
            break;
        nextToken();
    }
    accept('}');
    auto context = m_context;
    auto isStruct = m_isStruct;
    auto removeSymbols = m_removeSymbols;
    SymbolTableInfo info;
    info.size = offset;
    info.create = [context, name, comment, symbols, isStruct, removeSymbols](SymbolTable* symbolTable) {
        auto resultSymbolTable = symbolTable;
        if (!resultSymbolTable) {
            resultSymbolTable = new StandartSymbolTable(context);
        }
        if (!name.empty()) {
            resultSymbolTable->setName(name);
        }
        if (!comment.empty()) {
            resultSymbolTable->setComment(comment);
        }
        // add new symbols or change existing symbols
        auto symbolsToRemove = resultSymbolTable->getAllSymbols();
        for (auto& info : symbols) {
            auto symbol = resultSymbolTable->getSymbolAt(info.offset).symbol;
            auto dataType = info.createDataType();
            if (symbol) {
                symbolsToRemove.remove_if([&symbol](const SymbolTable::SymbolInfo& symbolInfo) {
                    return symbolInfo.symbol == symbol;
                });
            } else {
                if (isStruct) {
                    symbol = new StructureFieldSymbol(context);
                } else {
                    symbol = new VariableSymbol(context);
                }
                resultSymbolTable->addSymbol(info.offset, symbol);
            }
            symbol->setName(info.name);
            symbol->setComment(info.comment);
            symbol->setDataType(dataType);
        }
        // remove symbols
        if (removeSymbols) {
            for (auto& info : symbolsToRemove) {
                resultSymbolTable->removeSymbol(info.symbol->getOffset());
                info.symbol->destroy();
            }
        }
        return resultSymbolTable;
    };
    return info;
}

SymbolTableParser::SymbolInfo SymbolTableParser::parseSymbolDef() {
    auto comment = parseCommentIfExists();

    m_dataTypeParser->init(std::move(getToken()));
    auto symbolDtInfo = m_dataTypeParser->parseDataType();
    init(std::move(m_dataTypeParser->getToken()));

    std::string symbolName;
    if (!getToken()->isIdent(symbolName))
        throw error(200, "Expected field name");
    nextToken();

    auto context = m_context;
    SymbolInfo info;
    info.name = symbolName;
    info.comment = comment;
    info.size = symbolDtInfo.size;
    info.createDataType = symbolDtInfo.create;
    return info;
}