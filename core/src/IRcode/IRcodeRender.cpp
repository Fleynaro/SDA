#include "Core/IRcode/IRcodeRender.h"
#include "rang.hpp"

using namespace sda;
using namespace sda::ircode;

Render::Render(pcode::Render* pcodeRender)
    : m_pcodeRender(pcodeRender)
{}

void Render::renderOperation(const Operation* operation) {
    renderValue(operation->getOutput().get(), true);
    renderToken(" = ", Token::Other);
    renderToken(magic_enum::enum_name(operation->getId()).data(), Token::Operation);
    renderToken(" ", Token::Other);

    if (auto unaryOp = dynamic_cast<const UnaryOperation*>(operation)) {
        renderValue(unaryOp->getInput().get());
        if (auto extractOp = dynamic_cast<const ExtractOperation*>(operation)) {
            renderToken(", ", Token::Other);
            renderToken(std::to_string(extractOp->getOffset()), Token::Other);
        }
    }
    else if (auto binaryOp = dynamic_cast<const BinaryOperation*>(operation)) {
        renderValue(binaryOp->getInput1().get());
        renderToken(", ", Token::Other);
        renderValue(binaryOp->getInput2().get());
        if (auto concatOp = dynamic_cast<const ConcatOperation*>(operation)) {
            renderToken(", ", Token::Other);
            renderToken(std::to_string(concatOp->getOffset()), Token::Other);
        }
    }

    if (m_extendInfo) {
        if (operation->getOutput()->getLinearExpr().getTerms().size() > 1) {
            commenting(true);
            renderToken(" // ", Token::Comment);
            renderLinearExpr(&operation->getOutput()->getLinearExpr());
            commenting(false);
        }
    }
}

void Render::renderValue(const Value* value, bool extended) const {
    if (auto constValue = dynamic_cast<const Constant*>(value)) {
        m_pcodeRender->renderVarnode(constValue->getConstVarnode());
    }
    else if (auto regValue = dynamic_cast<const Register*>(value)) {
        m_pcodeRender->renderVarnode(regValue->getRegVarnode(), false);
    }
    else if (auto varValue = dynamic_cast<const Variable*>(value)) {
        auto varName = std::string("var") + std::to_string(varValue->getId());
        renderToken(varName, Token::Variable);
        if (extended) {
            renderToken("[", Token::Other);
            renderLinearExpr(&varValue->getMemAddress().value->getLinearExpr());
            renderToken("]:", Token::Other);
            renderToken(std::to_string(varValue->getSize()), Token::Other);
        }
    }
}

void Render::renderLinearExpr(const LinearExpression* linearExpr) const {
    const auto& terms = linearExpr->getTerms();
    for (auto it = terms.begin(); it != terms.end(); ++it) {
        if (it != terms.begin()) {
            renderToken(" + ", Token::Operation);
        }
        renderValue(it->value.get());
        if (it->factor != 1) {
            renderToken(" * ", Token::Other);
            renderToken(std::to_string(it->factor), Token::Other);
        }
    }
    auto constValue = linearExpr->getConstTermValue();
    if (constValue != 0) {
        renderToken(" + ", Token::Operation);
        renderToken(std::to_string(constValue), Token::Other);
    }
}

void Render::commenting(bool toggle) {
    m_commentingCounter += toggle ? 1 : -1;
    m_pcodeRender->commenting(toggle);
}

void Render::setExtendInfo(bool toggle) {
    m_extendInfo = toggle;
}

void Render::renderToken(const std::string& text, Token token) const {
    if (m_commentingCounter)
        token = Token::Comment;
    renderTokenImpl(text, token);
}

StreamRender::StreamRender(std::ostream& output, pcode::Render* pcodeRender)
    : Render(pcodeRender), m_output(output)
{}

void StreamRender::renderTokenImpl(const std::string& text, Token token) const {
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