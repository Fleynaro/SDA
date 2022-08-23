#include "Core/SymbolTable/SymbolTableParser.h"
#include "Core/SymbolTable/StandartSymbolTable.h"
#include "Core/Symbol/VariableSymbol.h"
#include "Core/Symbol/StructureFieldSymbol.h"
#include "Core/DataType/DataTypeParser.h"

using namespace sda;
using namespace utils::lexer;

SymbolTableParser::SymbolTableParser(utils::lexer::Lexer* lexer, Context* context, bool isStruct)
    : AbstractParser(lexer, 2), m_context(context), m_isStruct(isStruct)
{}

SymbolTable* SymbolTableParser::Parse(const std::string& text, Context* context, bool isStruct) {
    std::stringstream ss(text);
    IO io(ss, std::cout);
    Lexer lexer(&io);
    SymbolTableParser parser(&lexer, context, isStruct);
    parser.init();
    return parser.parse();
}

SymbolTable* SymbolTableParser::parse() {
    SymbolTable* symbolTable = new StandartSymbolTable(m_context);
    Offset offset = 0;

    if (!m_isStruct) {
        auto comment = parseCommentIfExists();
        
        std::string name;
        if (!getToken()->isIdent(name))
            throw error(100, "Expected symbol table name");
        nextToken();

        symbolTable->setName(name);
        symbolTable->setComment(comment);

        accept('=');
    }

    accept('{');
    while (!getToken()->isSymbol('}')) {
        // parse symbol
        auto symbol = parseSymbolDef();
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

        symbolTable->addSymbol(offset, symbol);
        offset += symbol->getDataType()->getSize();

        if (!getToken()->isSymbol(','))
            break;
        nextToken();
    }
    accept('}');
    return symbolTable;
}

Symbol* SymbolTableParser::parseSymbolDef() {
    DataTypeParser dataTypeParser(getLexer(), m_context);
    dataTypeParser.init(std::move(getToken()));
    auto symbolDt = dataTypeParser.parseDataType();
    init(std::move(dataTypeParser.getToken()));

    std::string symbolName;
    if (!getToken()->isIdent(symbolName))
        throw error(200, "Expected field name");
    nextToken();

    if (m_isStruct) {
        return new StructureFieldSymbol(
            m_context,
            nullptr,
            symbolName,
            symbolDt);
    }
    return new VariableSymbol(
        m_context,
        nullptr,
        symbolName,
        symbolDt);
}