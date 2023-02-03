#include "SDA/Core/Pcode/PcodePrinter.h"
#include <sstream>
#include "rang.hpp"
#include "SDA/Core/Utils/IOManip.h"

using namespace sda::pcode;

Printer::Printer(const RegisterRepository* regRepo)
    : m_regRepo(regRepo)
{}

std::string Printer::Print(const Instruction* instruction, const RegisterRepository* regRepo) {
    Printer printer(regRepo);
    std::stringstream ss;
    printer.setOutput(ss);
    printer.printInstruction(instruction);
    return ss.str();
}

void Printer::printFunctionGraph(FunctionGraph* functionGraph) {
    auto blocks = functionGraph->getBlocks();
    for (auto block : blocks) {
        printBlock(block);
        newLine();
    }
}

void Printer::printBlock(pcode::Block* block) {
    printToken("Block ", SYMBOL);
    printToken(block->getName(), IDENTIFIER);
    printToken("(level: ", SYMBOL);
    printToken(std::to_string(block->getLevel()), NUMBER);
    if (block->getNearNextBlock()) {
        printToken(", near:", SYMBOL);
        printToken(block->getNearNextBlock()->getName(), IDENTIFIER);
    }
    if (block->getFarNextBlock()) {
        printToken(", far:", SYMBOL);
        printToken(block->getFarNextBlock()->getName(), IDENTIFIER);
    }
    printToken("):", SYMBOL);
    startBlock();
    newLine();
    for (const auto& [offset, instruction] : block->getInstructions()) {
        printInstruction(instruction);
        newLine();
    }
    endBlock();
}

void Printer::printInstruction(const Instruction* instruction) const {
    if (instruction->getOutput()) {
        printVarnode(instruction->getOutput());
        printToken(" = ", SYMBOL);
    }

    printToken(magic_enum::enum_name(instruction->getId()).data(), MNEMONIC);

    if (instruction->getInput0()) {
        printToken(" ", SYMBOL);
        printVarnode(instruction->getInput0());
    }
    if (instruction->getInput1()) {
        printToken(", ", SYMBOL);
        printVarnode(instruction->getInput1());
    }
}

void Printer::printVarnode(std::shared_ptr<Varnode> varnode, bool printSizeAndOffset) const {
    if (auto regVarnode = std::dynamic_pointer_cast<RegisterVarnode>(varnode)) {
        const auto& reg = regVarnode->getRegister();
        auto regStr = reg.toString(m_regRepo, printSizeAndOffset);
        if (reg.getRegType() == Register::Virtual) {
            printToken(regStr, VIRT_REGISTER);
        } else {
            printToken(regStr, REGISTER);
        }
    }
    else if (auto constVarnode = std::dynamic_pointer_cast<ConstantVarnode>(varnode)) {
        std::stringstream ss;
        ss << "0x" << utils::to_hex() << constVarnode->getValue();
        if (printSizeAndOffset) 
            ss << ":" << constVarnode->getSize();
        printToken(ss.str(), NUMBER);
    }
}