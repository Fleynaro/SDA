#include "SDA/Core/Utils/AbstractPrinter.h"
#include "rang.hpp"

using namespace utils;

void AbstractPrinter::setOutput(std::ostream& output) {
    m_output = &output;
}

void AbstractPrinter::setTabSize(size_t tabSize) {
    m_tabSize = tabSize;
}

void AbstractPrinter::setParentPrinter(const AbstractPrinter* parent) {
    m_output = parent->m_output;
    m_tabSize = parent->m_tabSize;
    m_blockCounter = parent->m_blockCounter;
    m_commentingCounter = parent->m_commentingCounter;
}

void AbstractPrinter::startBlock() {
    m_blockCounter++;
}

void AbstractPrinter::endBlock() {
    m_blockCounter--;
}

void AbstractPrinter::startCommenting() {
    m_commentingCounter ++;
}

void AbstractPrinter::endCommenting() {
    m_commentingCounter --;
}

std::ostream& AbstractPrinter::out() const {
    if (!m_output)
        throw std::runtime_error("Output stream is not set");
    return *m_output;
}

void AbstractPrinter::newLine() const {
    printToken("\n", SPEC_SYMBOL);
    newTabs();
}

void AbstractPrinter::newTabs() const {
    for (size_t i = 0; i < m_blockCounter * m_tabSize; i++)
        printToken(" ", SYMBOL);
}

void AbstractPrinter::printComment(const std::string& text) const {
    printToken("[", SYMBOL);
    printToken("'", STRING);
    printToken(text, STRING);
    printToken("'", STRING);
    printToken("]", SYMBOL);
}

void AbstractPrinter::printToken(const std::string& text, Token token) const {
    if (m_commentingCounter)
        token = COMMENT;
    printTokenImpl(text, token);
}

void AbstractPrinter::printTokenImpl(const std::string& text, Token token) const {
    using namespace rang;
    switch (token) {
    case KEYWORD:
        out() << fg::blue;
        break;
    case IDENTIFIER:
        out() << fg::gray;
        break;
    case NUMBER:
        out() << fg::yellow;
        break;
    case STRING:
        out() << fg::magenta;
        break;
    case COMMENT:
        out() << fg::green;
        break;
    }
    out() << text << fg::reset;
}