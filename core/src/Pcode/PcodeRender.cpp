#include "Core/Pcode/PcodeRender.h"
#include <sstream>
#include "rang.hpp"
#include "Core/Utils/IOManip.h"

using namespace sda::pcode;

Render::Render(const PlatformSpec* platformSpec)
    : m_platformSpec(platformSpec)
{}

void Render::renderInstruction(const Instruction* instruction) const {
    if (instruction->getOutput()) {
        renderVarnode(instruction->getOutput().get());
        renderToken(" = ", Token::Other);
    }

    renderToken(magic_enum::enum_name(instruction->getId()).data(), Token::Mnemonic);

    if (instruction->getInput0()) {
        renderToken(" ", Token::Other);
        renderVarnode(instruction->getInput0().get());
    }
    if (instruction->getInput1()) {
        renderToken(", ", Token::Other);
        renderVarnode(instruction->getInput1().get());
    }
}

void Render::renderVarnode(const Varnode* varnode, bool renderSizeAndOffset) const {
    if (auto regVarnode = dynamic_cast<const RegisterVarnode*>(varnode)) {
        renderRegister(regVarnode->getRegister(), renderSizeAndOffset);
    }
    else if (auto constVarnode = dynamic_cast<const ConstantVarnode*>(varnode)) {
        std::stringstream ss;
        ss << "0x" << utils::to_hex() << constVarnode->getValue();
        if (renderSizeAndOffset) 
            ss << ":" << constVarnode->getSize();
        renderToken(ss.str(), Token::Number);
    }
}

void Render::renderRegister(const Register& reg, bool renderSizeAndOffset) const {
    auto size = reg.getSize();
    if (reg.getRegType() == Register::Virtual) {
        std::stringstream ss;
        ss << "$U" << (reg.getRegIndex() + 1);
        if (renderSizeAndOffset)
            ss << ":" << size;
        renderToken(ss.str(), Token::VirtRegister);
    } else {
        std::stringstream ss;
        if (reg.getRegType() == Register::Flag) {
            ss << m_platformSpec->getRegisterFlagName(reg.getMask());
        } else {
            if (reg.getRegType() == Register::StackPointer)
                ss << "RSP";
            else if (reg.getRegType() == Register::InstructionPointer)
                ss << "RIP";
            else 
                ss << m_platformSpec->getRegisterName(reg.getRegId());
        }

        if (renderSizeAndOffset) {
            if (reg.getRegType() == Register::Vector) {
                if (size == 4 || size == 8) {
                    ss << (size == 4 ? "D" : "Q");
                    ss << static_cast<char>('a' + static_cast<char>(reg.getOffset() / (size * BitsInBytes)));
                }
            } else {
                ss << ":" << size;
            }
        }

        renderToken(ss.str(), Token::Register);
    }
}

void Render::commenting(bool toggle) {
    m_commentingCounter += toggle ? 1 : -1;
}

void Render::renderToken(const std::string& text, Token token) const {
    if (m_commentingCounter)
        token = Token::Comment;
    renderTokenImpl(text, token);
}

StreamRender::StreamRender(std::ostream& output, const PlatformSpec* platformSpec)
    : Render(platformSpec), m_output(output)
{}

void StreamRender::renderTokenImpl(const std::string& text, Token token) const {
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