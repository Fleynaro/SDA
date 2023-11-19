#include "SDA/Core/SymbolTable/SymbolTablePrinter.h"
#include "SDA/Core/Symbol/Symbol.h"
#include "SDA/Core/Utils/String.h"
#include "rang.hpp"

using namespace sda;

SymbolTablePrinter::SymbolTablePrinter(Context* context)
    : m_context(context)
{}

std::string SymbolTablePrinter::Print(SymbolTable* symbolTable, Context* context, bool withName) {
    SymbolTablePrinter printer(context);
    std::stringstream ss;
    printer.setOutput(ss);
    printer.printDef(symbolTable, withName);
    return ss.str();
}

void SymbolTablePrinter::printDef(SymbolTable* symbolTable, bool withName) {
    if (withName) {
        if (!symbolTable->getComment().empty()) {
            printComment(symbolTable->getComment());
            newLine();
        }
        printToken(symbolTable->getName(), IDENTIFIER);
        printToken(" = ", SYMBOL);
    }

    printToken("{", SYMBOL);
    startBlock();
    Offset offset = 0;
    auto symbols = symbolTable->getAllSymbols();
    for (auto& symbolInfo : symbols) {
        newLine();
        auto symbol = symbolInfo.symbol;
        printDataType(symbol->getDataType());
        printToken(" ", SYMBOL);
        printToken(symbol->getName(), IDENTIFIER);
        if (symbol->getOffset() != offset) {
            printToken(" = ", SYMBOL);
            printToken("0x" + utils::ToHex(symbol->getOffset()), NUMBER);
            offset = symbol->getOffset();
        }
        if (symbol != symbols.back().symbol)
            printToken(", ", SYMBOL);
        offset += symbol->getDataType()->getSize();
    }
    endBlock();
    newLine();
    printToken("}", SYMBOL);
}

void SymbolTablePrinter::printDataType(DataType* dataType) {
    printToken(dataType->getName(), DATATYPE);
}

void SymbolTablePrinter::printTokenImpl(const std::string& text, Token token) const {
    switch (token) {
    case DATATYPE:
        out() << rang::fg::yellow << text;
        break;
    default:
        AbstractPrinter::printTokenImpl(text, token);
    }
    out() << rang::fg::reset;
}