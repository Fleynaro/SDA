#include "SDA/Core/IRcode/IRcodePrinter.h"
#include "rang.hpp"

using namespace sda;
using namespace sda::ircode;

Printer::Printer(pcode::Printer* pcodePrinter)
    : m_pcodePrinter(pcodePrinter)
{}

void Printer::setOperationCommentProvider(const OperationCommentProvider& operationCommentProvider) {
    m_operationCommentProvider = operationCommentProvider;
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
        printValue(block->getCondition());
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

void Printer::printOperation(const Operation* operation) {
    auto output = operation->getOutput();
    printValue(output, true);
    printToken(" = ", SYMBOL);
    printToken(magic_enum::enum_name(operation->getId()).data(), OPERATION);
    printToken(" ", SYMBOL);

    if (auto unaryOp = dynamic_cast<const UnaryOperation*>(operation)) {
        printValue(unaryOp->getInput());
        if (auto extractOp = dynamic_cast<const ExtractOperation*>(operation)) {
            printToken(", ", SYMBOL);
            printToken(std::to_string(extractOp->getOffset()), SYMBOL);
        }
    }
    else if (auto binaryOp = dynamic_cast<const BinaryOperation*>(operation)) {
        printValue(binaryOp->getInput1());
        printToken(", ", SYMBOL);
        printValue(binaryOp->getInput2());
        if (auto concatOp = dynamic_cast<const ConcatOperation*>(operation)) {
            printToken(", ", SYMBOL);
            printToken(std::to_string(concatOp->getOffset()), SYMBOL);
        }
    }
    else if (auto callOp = dynamic_cast<const CallOperation*>(operation)) {
        printValue(callOp->getDestination());
        for (auto input : callOp->getArguments()) {
            printToken(", ", SYMBOL);
            printValue(input);
        }
    }

    if (m_operationCommentProvider) {
        auto comment = m_operationCommentProvider(operation);
        if (!comment.empty()) {
            startCommenting();
            printToken(" // ", COMMENT);
            printToken(m_operationCommentProvider(operation), COMMENT);
            endCommenting();
        }
    }
}

void Printer::printValue(std::shared_ptr<Value> value, bool extended) {
    if (auto constValue = dynamic_cast<const Constant*>(value.get())) {
        m_pcodePrinter->setParentPrinter(this);
        m_pcodePrinter->printVarnode(constValue->getConstVarnode());
    }
    else if (auto regValue = dynamic_cast<const Register*>(value.get())) {
        m_pcodePrinter->setParentPrinter(this);
        m_pcodePrinter->printVarnode(regValue->getRegVarnode(), false);
    }
    else if (auto varValue = dynamic_cast<const Variable*>(value.get())) {
        printToken(varValue->getName(), VARIABLE);
        if (extended) {
            if (!varValue->getMemAddress().isVirtual || m_printVarAddressAlways) {
                auto memAddressValue = varValue->getMemAddress().value;
                if (memAddressValue) {
                    printToken("[", SYMBOL);
                    printValue(memAddressValue);
                    printToken("]", SYMBOL);
                }
            }
            printToken(":", SYMBOL);
            printToken(std::to_string(varValue->getSize()), SYMBOL);
        }
    }
}

void Printer::printLinearExpr(const LinearExpression* linearExpr) {
    const auto& terms = linearExpr->getTerms();
    for (auto it = terms.begin(); it != terms.end(); ++it) {
        if (it != terms.begin()) {
            printToken(" + ", OPERATION);
        }
        printValue(it->value);
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

pcode::StructTreePrinter::PrinterFunction Printer::getCodePrinter(Function* function) {
    return std::function([this, function](pcode::Block* pcodeBlock) {
        auto block = function->toBlock(pcodeBlock);
        if (!block) return;
        printToken("// Block ", Printer::COMMENT);
        printToken(block->getName(), Printer::COMMENT);

        auto& operations = block->getOperations();
        if (!operations.empty())
            newLine();
        for (auto it = operations.begin(); it != operations.end(); ++it) {
            printOperation(it->get());
            if (it != std::prev(operations.end()))
                newLine();
        }
    });
}

pcode::StructTreePrinter::PrinterFunction Printer::getConditionPrinter(Function* function) {
    return std::function([this, function](pcode::Block* pcodeBlock) {
        auto block = function->toBlock(pcodeBlock);
        if (!block) return;
        if (block->getCondition()) {
            printValue(block->getCondition());
        }
    });
}