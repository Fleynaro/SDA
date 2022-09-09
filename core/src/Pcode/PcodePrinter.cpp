#include "Core/Pcode/PcodePrinter.h"
#include <sstream>
#include "rang.hpp"
#include "Core/Utils/IOManip.h"

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

void Printer::printInstruction(const Instruction* instruction) const {
    if (instruction->getOutput()) {
        printVarnode(instruction->getOutput().get());
        printToken(" = ", SYMBOL);
    }

    printToken(magic_enum::enum_name(instruction->getId()).data(), MNEMONIC);

    if (instruction->getInput0()) {
        printToken(" ", SYMBOL);
        printVarnode(instruction->getInput0().get());
    }
    if (instruction->getInput1()) {
        printToken(", ", SYMBOL);
        printVarnode(instruction->getInput1().get());
    }
}

void Printer::printVarnode(const Varnode* varnode, bool printSizeAndOffset) const {
    if (auto regVarnode = dynamic_cast<const RegisterVarnode*>(varnode)) {
        const auto& reg = regVarnode->getRegister();
        auto regStr = reg.toString(m_regRepo, printSizeAndOffset);
        if (reg.getRegType() == Register::Virtual) {
            printToken(regStr, VIRT_REGISTER);
        } else {
            printToken(regStr, REGISTER);
        }
    }
    else if (auto constVarnode = dynamic_cast<const ConstantVarnode*>(varnode)) {
        std::stringstream ss;
        ss << "0x" << utils::to_hex() << constVarnode->getValue();
        if (printSizeAndOffset) 
            ss << ":" << constVarnode->getSize();
        printToken(ss.str(), NUMBER);
    }
}