#include "Core/IRcode/IRcodePrinter.h"
#include "rang.hpp"

using namespace sda;
using namespace sda::ircode;

Printer::Printer(pcode::Printer* pcodePrinter)
    : m_pcodePrinter(pcodePrinter)
{}

void Printer::setDataTypeProvider(DataTypeProvider* dataTypeProvider) {
    m_dataTypeProvider = dataTypeProvider;
}

void Printer::printOperation(Operation* operation) {
    auto output = operation->getOutput();
    printValue(output.get(), true);
    printToken(" = ", Token::Other);
    printToken(magic_enum::enum_name(operation->getId()).data(), Token::Operation);
    printToken(" ", Token::Other);

    if (auto unaryOp = dynamic_cast<const UnaryOperation*>(operation)) {
        printValue(unaryOp->getInput().get());
        if (auto extractOp = dynamic_cast<const ExtractOperation*>(operation)) {
            printToken(", ", Token::Other);
            printToken(std::to_string(extractOp->getOffset()), Token::Other);
        }
    }
    else if (auto binaryOp = dynamic_cast<const BinaryOperation*>(operation)) {
        printValue(binaryOp->getInput1().get());
        printToken(", ", Token::Other);
        printValue(binaryOp->getInput2().get());
        if (auto concatOp = dynamic_cast<const ConcatOperation*>(operation)) {
            printToken(", ", Token::Other);
            printToken(std::to_string(concatOp->getOffset()), Token::Other);
        }
    }

    if (m_extendInfo) {
        if (m_dataTypeProvider) {
            if (auto dataType = m_dataTypeProvider->getDataType(output)) {
                commenting(true);
                printToken(" // ", Token::Comment);
                printToken(dataType->getName(), Token::Comment);
                commenting(false);
            }
        }

        const auto& terms = output->getLinearExpr().getTerms();
        if (!(terms.size() == 1 && terms.front().factor == 1)) {
            commenting(true);
            printToken(" // ", Token::Comment);
            printLinearExpr(&output->getLinearExpr());
            commenting(false);
        }

        const auto& vars = operation->getOverwrittenVariables();
        if (!vars.empty()) {
            commenting(true);
            printToken(" // overwrites ", Token::Comment);
            for (const auto& var : vars) {
                printValue(var.get());
                if (var != *vars.rbegin())
                    printToken(", ", Token::Comment);
            }
            commenting(false);
        }
    }
}

void Printer::printValue(const Value* value, bool extended) const {
    if (auto constValue = dynamic_cast<const Constant*>(value)) {
        m_pcodePrinter->printVarnode(constValue->getConstVarnode());
    }
    else if (auto regValue = dynamic_cast<const Register*>(value)) {
        m_pcodePrinter->printVarnode(regValue->getRegVarnode(), false);
    }
    else if (auto varValue = dynamic_cast<const Variable*>(value)) {
        printToken(varValue->getName(), Token::Variable);
        if (extended) {
            printToken("[", Token::Other);
            printLinearExpr(&varValue->getMemAddress().value->getLinearExpr());
            printToken("]:", Token::Other);
            printToken(std::to_string(varValue->getSize()), Token::Other);
        }
    }
}

void Printer::printLinearExpr(const LinearExpression* linearExpr) const {
    const auto& terms = linearExpr->getTerms();
    for (auto it = terms.begin(); it != terms.end(); ++it) {
        if (it != terms.begin()) {
            printToken(" + ", Token::Operation);
        }
        printValue(it->value.get());
        if (it->factor != 1) {
            printToken(" * ", Token::Other);
            printToken(std::to_string(it->factor), Token::Other);
        }
    }
    auto constValue = linearExpr->getConstTermValue();
    if (constValue != 0) {
        printToken(" + ", Token::Operation);
        printToken(std::to_string(constValue), Token::Other);
    }
}

void Printer::commenting(bool toggle) {
    m_commentingCounter += toggle ? 1 : -1;
    m_pcodePrinter->commenting(toggle);
}

void Printer::setExtendInfo(bool toggle) {
    m_extendInfo = toggle;
}

void Printer::printToken(const std::string& text, Token token) const {
    if (m_commentingCounter)
        token = Token::Comment;
    printTokenImpl(text, token);
}

StreamPrinter::StreamPrinter(std::ostream& output, pcode::Printer* pcodePrinter)
    : Printer(pcodePrinter), m_output(output)
{}

void StreamPrinter::printTokenImpl(const std::string& text, Token token) const {
    using namespace rang;
    switch (token) {
    case Token::Operation:
        m_output << fg::yellow;
        break;
    case Token::Variable:
        m_output << fg::gray;
        break;
    case Token::Comment:
        m_output << fg::green;
        break;
    }
    m_output << text << fg::reset;
}