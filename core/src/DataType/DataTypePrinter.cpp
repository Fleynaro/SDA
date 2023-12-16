#include "SDA/Core/DataType/DataTypePrinter.h"
#include "SDA/Core/DataType/VoidDataType.h"
#include "SDA/Core/DataType/PointerDataType.h"
#include "SDA/Core/DataType/ArrayDataType.h"
#include "SDA/Core/DataType/ScalarDataType.h"
#include "SDA/Core/DataType/TypedefDataType.h"
#include "SDA/Core/DataType/EnumDataType.h"
#include "SDA/Core/DataType/StructureDataType.h"
#include "SDA/Core/DataType/SignatureDataType.h"
#include "SDA/Core/SymbolTable/SymbolTablePrinter.h"
#include "rang.hpp"

using namespace sda;

DataTypePrinter::DataTypePrinter(Context* context, SymbolTablePrinter* symbolTablePrinter)
    : m_context(context), m_symbolTablePrinter(symbolTablePrinter)
{}

std::string DataTypePrinter::Print(const std::list<ParsedDataType>& parsedDataTypes, Context* context, bool withName) {
    SymbolTablePrinter symbolTablePrinter(context);
    DataTypePrinter printer(context, &symbolTablePrinter);
    std::stringstream ss;
    printer.setOutput(ss);
    for (auto& [dataType, isDeclared] : parsedDataTypes) {
        printer.printDef(dataType, withName, !isDeclared);
        printer.newLine();
    }
    return ss.str();
}

void DataTypePrinter::printDef(DataType* dataType, bool withName, bool withBody) {
    if (withName) {
        if (!dataType->getComment().empty()) {
            printComment(dataType->getComment());
            newLine();
        }
        printToken(dataType->getName(), IDENTIFIER);
        printToken(" = ", SYMBOL);
    }
    if (auto typedefDt = dynamic_cast<TypedefDataType*>(dataType)) {
        printTypeDef(typedefDt);
    } else if (auto enumDt = dynamic_cast<EnumDataType*>(dataType)) {
        printEnumDef(enumDt);
    } else if (auto structDt = dynamic_cast<StructureDataType*>(dataType)) {
        printStructureDef(structDt, withBody);
    } else if (auto signatureDt = dynamic_cast<SignatureDataType*>(dataType)) {
        printSignatureDef(signatureDt);
    } else {
        throw std::runtime_error("Unknown data type");
    }
}

void DataTypePrinter::printTypeDef(TypedefDataType* typedefDt) {
    printToken("typedef ", KEYWORD);
    printDataType(typedefDt->getReferenceType());
}

void DataTypePrinter::printEnumDef(EnumDataType* enumDt) {
    printToken("enum ", KEYWORD);
    printToken("{", SYMBOL);
    startBlock();
    size_t fieldIdx = 0;
    auto fields = enumDt->getFields();
    for (auto& [key, name] : fields) {
        newLine();
        printToken(name, IDENTIFIER);
        if (key != fieldIdx) {
            printToken(" = ", SYMBOL);
            printToken(std::to_string(key), NUMBER);
            fieldIdx = key;
        }
        if (key != fields.rbegin()->first)
            printToken(", ", SYMBOL);
        fieldIdx++;
    }
    endBlock();
    newLine();
    printToken("}", SYMBOL);
}

void DataTypePrinter::printStructureDef(StructureDataType* structDt, bool withBody) {
    printToken("struct" , KEYWORD);
    if (withBody) {
        printToken(" " , SYMBOL);
        m_symbolTablePrinter->setParentPrinter(this);
        m_symbolTablePrinter->printDef(structDt->getSymbolTable(), false);
    }
}

void DataTypePrinter::printSignatureDef(SignatureDataType* signatureDt) {
    printToken("signature ", KEYWORD);
    printToken(signatureDt->getCallingConvention()->getName(), KEYWORD);
    printToken(" ", SYMBOL);
    printDataType(signatureDt->getReturnType());
    printToken(" (", SYMBOL);
    auto parameters = signatureDt->getParameters();
    for (auto paramSymbol : parameters) {
        printDataType(paramSymbol->getDataType());
        printToken(" ", SYMBOL);
        printToken(paramSymbol->getName(), IDENTIFIER);
        if (paramSymbol != parameters.back())
            printToken(", ", SYMBOL);
    }
    printToken(")", SYMBOL);
}

void DataTypePrinter::printDataType(DataType* dataType) {
    printToken(dataType->getName(), DATATYPE);
}

void DataTypePrinter::printTokenImpl(const std::string& text, Token token) const {
    switch (token) {
    case DATATYPE:
        out() << rang::fg::yellow << text;
        break;
    default:
        AbstractPrinter::printTokenImpl(text, token);
    }
    out() << rang::fg::reset;
}