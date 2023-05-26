#include "SDA/Core/IRcode/IRcodePrinter.h"
#include "rang.hpp"

using namespace sda;
using namespace sda::ircode;

Printer::Printer(pcode::Printer* pcodePrinter)
    : m_pcodePrinter(pcodePrinter)
{}

void Printer::setDataTypeProvider(DataTypeProvider* dataTypeProvider) {
    m_dataTypeProvider = dataTypeProvider;
}

void Printer::setExtendInfo(bool toggle) {
    m_extendInfo = toggle;
}

void Printer::printFunction(Function* function) {
    auto blockInfos = function->getFunctionGraph()->getBlocks(true);
    // print blocks
    for (auto& [block, level] : blockInfos) {
        printBlock(function->toBlock(block), level);
        if (block != blockInfos.back().block)
            newLine();
    }
}

void Printer::printBlock(Block* block, size_t level) {
    printToken("Block ", SYMBOL);
    printToken(block->getName(), IDENTIFIER);
    printToken("(level: ", SYMBOL);
    printToken(std::to_string(level), NUMBER);
    if (block->getNearNextBlock()) {
        printToken(", near: ", SYMBOL);
        printToken(block->getNearNextBlock()->getName(), IDENTIFIER);
    }
    if (block->getFarNextBlock()) {
        printToken(", far: ", SYMBOL);
        printToken(block->getFarNextBlock()->getName(), IDENTIFIER);
    }
    if (block->getCondition()) {
        printToken(", cond: ", SYMBOL);
        printValue(block->getCondition().get());
    }
    printToken("):", SYMBOL);
    startBlock();
    newLine();
    if (block->getOperations().empty()) {
        printToken("empty", SYMBOL);
    } else {
        for (const auto& operation : block->getOperations()) {
            printOperation(operation.get());
            if (operation != *block->getOperations().rbegin())
                newLine();
        }
    }
    endBlock();
}

void Printer::printOperation(Operation* operation) {
    auto output = operation->getOutput();
    printValue(output.get(), true);
    printToken(" = ", SYMBOL);
    printToken(magic_enum::enum_name(operation->getId()).data(), OPERATION);
    printToken(" ", SYMBOL);

    if (auto unaryOp = dynamic_cast<const UnaryOperation*>(operation)) {
        printValue(unaryOp->getInput().get());
        if (auto extractOp = dynamic_cast<const ExtractOperation*>(operation)) {
            printToken(", ", SYMBOL);
            printToken(std::to_string(extractOp->getOffset()), SYMBOL);
        }
    }
    else if (auto binaryOp = dynamic_cast<const BinaryOperation*>(operation)) {
        printValue(binaryOp->getInput1().get());
        printToken(", ", SYMBOL);
        printValue(binaryOp->getInput2().get());
        if (auto concatOp = dynamic_cast<const ConcatOperation*>(operation)) {
            printToken(", ", SYMBOL);
            printToken(std::to_string(concatOp->getOffset()), SYMBOL);
        }
    }

    if (m_extendInfo) {
        if (m_dataTypeProvider) {
            if (auto dataType = m_dataTypeProvider->getDataType(output)) {
                startCommenting();
                printToken(" // ", COMMENT);
                printToken(dataType->getName(), COMMENT);
                endCommenting();
            }
        }

        const auto& terms = output->getLinearExpr().getTerms();
        if (!(terms.size() == 1 && terms.front().factor == 1)) {
            startCommenting();
            printToken(" // ", COMMENT);
            printLinearExpr(&output->getLinearExpr());
            endCommenting();
        }

        const auto& vars = operation->getOverwrittenVariables();
        if (!vars.empty()) {
            startCommenting();
            printToken(" // overwrites ", COMMENT);
            for (const auto& var : vars) {
                printValue(var.get());
                if (var != *vars.rbegin())
                    printToken(", ", COMMENT);
            }
            endCommenting();
        }
    }
}

void Printer::printValue(const Value* value, bool extended) const {
    if (auto constValue = dynamic_cast<const Constant*>(value)) {
        m_pcodePrinter->setParentPrinter(this);
        m_pcodePrinter->printVarnode(constValue->getConstVarnode());
    }
    else if (auto regValue = dynamic_cast<const Register*>(value)) {
        m_pcodePrinter->setParentPrinter(this);
        m_pcodePrinter->printVarnode(regValue->getRegVarnode(), false);
    }
    else if (auto varValue = dynamic_cast<const Variable*>(value)) {
        printToken(varValue->getName(), VARIABLE);
        if (extended) {
            auto memAddressValue = varValue->getMemAddress().value;
            if (memAddressValue) {
                printToken("[", SYMBOL);
                printValue(memAddressValue.get());
                printToken("]", SYMBOL);
            }
            printToken(":", SYMBOL);
            printToken(std::to_string(varValue->getSize()), SYMBOL);
        }
    }
}

void Printer::printLinearExpr(const LinearExpression* linearExpr) const {
    const auto& terms = linearExpr->getTerms();
    for (auto it = terms.begin(); it != terms.end(); ++it) {
        if (it != terms.begin()) {
            printToken(" + ", OPERATION);
        }
        printValue(it->value.get());
        if (it->factor != 1) {
            printToken(" * ", SYMBOL);
            printToken(std::to_string(it->factor), SYMBOL);
        }
    }
    auto constValue = linearExpr->getConstTermValue();
    if (constValue != 0) {
        printToken(" + ", OPERATION);
        printToken(std::to_string(constValue), SYMBOL);
    }
}