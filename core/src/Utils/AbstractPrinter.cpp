#include "SDA/Core/Utils/AbstractPrinter.h"
#include <boost/algorithm/string.hpp>
#include "rang.hpp"

using namespace utils;

void AbstractPrinter::setOutput(std::ostream& output) {
    m_output = &output;
}

void AbstractPrinter::setTabSize(size_t tabSize) {
    m_tabSize = tabSize;
}

void AbstractPrinter::setParentPrinter(AbstractPrinter* parent) {
    m_parentPrinter = parent;
}

void AbstractPrinter::startBlock() {
    if (m_parentPrinter) {
        m_parentPrinter->startBlock();
        return;
    }
    m_blockCounter++;
}

void AbstractPrinter::endBlock() {
    if (m_parentPrinter) {
        m_parentPrinter->endBlock();
        return;
    }
    m_blockCounter--;
}

void AbstractPrinter::startCommenting() {
    if (m_parentPrinter) {
        m_parentPrinter->startCommenting();
        return;
    }
    m_commentingCounter ++;
}

void AbstractPrinter::endCommenting() {
    if (m_parentPrinter) {
        m_parentPrinter->endCommenting();
        return;
    }
    m_commentingCounter --;
}

std::ostream& AbstractPrinter::out() const {
    if (m_parentPrinter) {
        return m_parentPrinter->out();
    }
    if (!m_output)
        throw std::runtime_error("Output stream is not set");
    return *m_output;
}

void AbstractPrinter::newLine() const {
    printToken("\n", SPEC_SYMBOL);
    newTabs();
}

void AbstractPrinter::newTabs() const {
    auto blockCounter = m_parentPrinter ? m_parentPrinter->m_blockCounter : m_blockCounter;
    auto tabSize = m_parentPrinter ? m_parentPrinter->m_tabSize : m_tabSize;
    for (size_t i = 0; i < blockCounter * tabSize; i++)
        printToken(" ", SYMBOL);
}

void AbstractPrinter::printComment(const std::string& text) const {
    std::vector<std::string> lines;
    boost::split(lines, text, boost::is_any_of("\n"));
    for (size_t i = 0; i < lines.size(); i++) {
        if (i > 0) newLine();
        printToken("// ", COMMENT);
        printToken(lines[i], COMMENT);
    }
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