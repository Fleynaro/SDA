#include "Core/SymbolTable/SymbolTablePrinter.h"
#include "Core/Symbol/Symbol.h"
#include "Core/Utils/IOManip.h"
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
        if (symbolInfo.symbolOffset != offset) {
            printToken(" = ", SYMBOL);
            auto offsetStr = (
                std::stringstream() << "0x" << utils::to_hex() << symbolInfo.symbolOffset).str();
            printToken(offsetStr, NUMBER);
            offset = symbolInfo.symbolOffset;
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