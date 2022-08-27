#include "Core/Pcode/PcodePrinter.h"
#include <sstream>
#include "rang.hpp"
#include "Core/Utils/IOManip.h"

using namespace sda::pcode;

Printer::Printer(const RegisterRepository* regRepo)
    : m_regRepo(regRepo)
{}

void Printer::printInstruction(const Instruction* instruction) const {
    if (instruction->getOutput()) {
        printVarnode(instruction->getOutput().get());
        printToken(" = ", Token::Other);
    }

    printToken(magic_enum::enum_name(instruction->getId()).data(), Token::Mnemonic);

    if (instruction->getInput0()) {
        printToken(" ", Token::Other);
        printVarnode(instruction->getInput0().get());
    }
    if (instruction->getInput1()) {
        printToken(", ", Token::Other);
        printVarnode(instruction->getInput1().get());
    }
}

void Printer::printVarnode(const Varnode* varnode, bool printSizeAndOffset) const {
    if (auto regVarnode = dynamic_cast<const RegisterVarnode*>(varnode)) {
        const auto& reg = regVarnode->getRegister();
        auto regStr = reg.toString(m_regRepo, printSizeAndOffset);
        if (reg.getRegType() == Register::Virtual) {
            printToken(regStr, Token::VirtRegister);
        } else {
            printToken(regStr, Token::Register);
        }
    }
    else if (auto constVarnode = dynamic_cast<const ConstantVarnode*>(varnode)) {
        std::stringstream ss;
        ss << "0x" << utils::to_hex() << constVarnode->getValue();
        if (printSizeAndOffset) 
            ss << ":" << constVarnode->getSize();
        printToken(ss.str(), Token::Number);
    }
}

void Printer::commenting(bool toggle) {
    m_commentingCounter += toggle ? 1 : -1;
}

void Printer::printToken(const std::string& text, Token token) const {
    if (m_commentingCounter)
        token = Token::Comment;
    printTokenImpl(text, token);
}

StreamPrinter::StreamPrinter(std::ostream& output, const RegisterRepository* regRepo)
    : Printer(regRepo), m_output(output)
{}

void StreamPrinter::printTokenImpl(const std::string& text, Token token) const {
    using namespace rang;
    switch (token) {
    case Token::Mnemonic:
        m_output << fg::yellow;
        break;
    case Token::Register:
    case Token::VirtRegister:
        m_output << fg::cyan;
        break;
    case Token::Number:
        m_output << fg::blue;
        break;
    case Token::Comment:
        m_output << fg::green;
        break;
    }
    m_output << text << fg::reset;
}