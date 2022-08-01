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
        auto size = regVarnode->getSize();
        if (regVarnode->getRegType() == RegisterVarnode::Virtual) {
            std::stringstream ss;
            ss << "$U" << (regVarnode->getRegIndex() + 1);
            if (renderSizeAndOffset)
                ss << ":" << size;
            renderToken(ss.str(), Token::VirtRegister);
        } else {
            std::stringstream ss;
            if (regVarnode->getRegType() == pcode::RegisterVarnode::Flag) {
                ss << m_platformSpec->getRegisterFlagName(varnode->getMask());
            } else {
                if (regVarnode->getRegType() == pcode::RegisterVarnode::StackPointer)
                    ss << "RSP";
                else if (regVarnode->getRegType() == pcode::RegisterVarnode::InstructionPointer)
                    ss << "RIP";
                else 
                    ss << m_platformSpec->getRegisterName(regVarnode->getRegId());
            }

            if (renderSizeAndOffset) {
                if (regVarnode->getRegType() == pcode::RegisterVarnode::Vector) {
                    if (size == 4 || size == 8) {
                        ss << (size == 4 ? "D" : "Q");
                        ss << static_cast<char>('a' + static_cast<char>(regVarnode->getOffset() / (size * BitsInBytes)));
                    }
                } else {
                    ss << ":" << size;
                }
            }

            renderToken(ss.str(), Token::Register);
        }
    }
    else if (auto constVarnode = dynamic_cast<const ConstantVarnode*>(varnode)) {
        std::stringstream ss;
        ss << "0x" << utils::to_hex() << constVarnode->getValue();
        if (renderSizeAndOffset) 
            ss << ":" << constVarnode->getSize();
        renderToken(ss.str(), Token::Number);
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