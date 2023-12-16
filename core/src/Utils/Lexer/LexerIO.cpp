#include "SDA/Core/Utils/Lexer/LexerIO.h"
#include "rang.hpp"

using namespace utils::lexer;

IO::IO(std::istream& streamIn, std::ostream& streamOut)
    : m_streamIn(streamIn), m_streamOut(streamOut)
{}

char IO::nextLetter() {
    // if we are at the end of the line, read the next line
    if (m_letterPos.m_column == m_curLine.length()) {
        if (!readNextLine()) {
            if (m_errorsCount > 0) {
                m_curLineShown = true;
                message(std::to_string(m_errorsCount) + " errors");
            }
            return '\0';
        }
    }
    // get the next letter
    m_letter = m_curLine[m_letterPos.m_column++];
    return m_letter;
}

void IO::error(const ErrorCode errorCode, const std::string& text, size_t colPos) {
    const auto MAX_ERRORS = 100;
    const auto MAX_ERRORS_ON_LINE = 5;

    if (colPos == -1)
        colPos = m_letterPos.m_column;

    // ignore the same error
    if (errorCode == m_lastErrorCode && colPos == m_lastErrorColPos)
        return;

    if (m_errorsCount < MAX_ERRORS) {
        listCurLineOnce();

        if (m_errorsCountOnLine < MAX_ERRORS_ON_LINE) {
            m_streamOut << rang::fg::red;
            m_streamOut << "**" << std::setfill('0') << std::setw(2) << (m_errorsCount + 1) << "**";
            for (int i = 0; i < int(colPos) - 2; ++i)
                m_streamOut << " ";

            m_streamOut << rang::fg::red;
            m_streamOut << "^ error code " << errorCode << std::endl;
            m_streamOut << "******  " << text << std::endl;

            m_lastErrorCode = errorCode;
            m_lastErrorColPos = colPos;
        }
        else  if (m_errorsCountOnLine == MAX_ERRORS_ON_LINE) {
            m_streamOut << rang::fg::red;
            m_streamOut << "******  " << "Too many errors on this line" << std::endl;
        }

        m_streamOut << rang::fg::reset;
    }

    m_errorsCount++;
    m_errorsCountOnLine++;
}

void IO::message(const std::string& text) {
    listCurLineOnce();
    m_streamOut << "@@@@@@  " << text << std::endl;
}

size_t IO::getColumnPos() const {
    return m_letterPos.m_column;
}

bool IO::hasErrors() const {
    return m_errorsCount > 0;
}

void IO::listCurLineOnce() {
    if (m_curLineShown)
        return;
    auto line = m_curLine;
    if (line.empty()) {
        line = "<Empty line>";
    } else {
        line.pop_back();
    }

    m_streamOut << "  " << std::setfill(' ') << std::setw(2) << m_letterPos.m_line << "  ";
    m_streamOut << line << std::endl;
    m_curLineShown = true;
}

bool IO::readNextLine() {
    m_letterPos.m_column = 0;
    m_letterPos.m_line++;
    m_errorsCountOnLine = 0;
    m_curLineShown = false;
    m_curLine.clear();
    do {
        // if reach end of file
        if (m_streamIn.eof()) {
            return false;
        }
        std::getline(m_streamIn, m_curLine);
    } while (m_curLine.empty());
    m_curLine += '\n';
    return true;
}