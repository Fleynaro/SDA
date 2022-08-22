#include "Core/Pcode/PcodeRender.h"
#include <sstream>
#include "rang.hpp"
#include "Core/Utils/IOManip.h"

using namespace sda::pcode;

Render::Render(const RegisterRepository* regRepo)
    : m_regRepo(regRepo)
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
        const auto& reg = regVarnode->getRegister();
        auto regStr = reg.toString(m_regRepo, renderSizeAndOffset);
        if (reg.getRegType() == Register::Virtual) {
            renderToken(regStr, Token::VirtRegister);
        } else {
            renderToken(regStr, Token::Register);
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

StreamRender::StreamRender(std::ostream& output, const RegisterRepository* regRepo)
    : Render(regRepo), m_output(output)
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