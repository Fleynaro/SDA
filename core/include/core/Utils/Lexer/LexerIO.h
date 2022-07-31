#pragma once
#include <iostream>
#include <iomanip>
#include <string>

namespace utils::lexer
{
    using ErrorCode = size_t;

    // A lexer IO class for reading and writing to files/console/...
    class IO {
        struct LetterPosition {
            size_t m_line = 0;
            size_t m_column = 0;
        };

        std::istream* m_streamIn;
        std::ostream* m_streamOut;
        std::string m_curLine;
        std::size_t m_errorsCount = 0;
        std::size_t m_errorsCountOnLine = 0;
        ErrorCode m_lastErrorCode = -1;
        size_t m_lastErrorColPos = -1;
        bool m_curLineShown = false;

        char m_letter;
        LetterPosition m_letterPos;
    public:
        IO(std::istream* streamIn, std::ostream* streamOut);

        // Reads the next letter from the input stream.
        char nextLetter();

        // Writes an error message to the output stream.
        void error(const ErrorCode errorCode, const std::string& text, size_t colPos = -1);

        // Writes a message to the output stream.
        void message(const std::string& text);

        // Get the current column position.
        size_t getColumnPos() const;

        // Check if errors were found.
        bool hasErrors() const;

    private:
        void listCurLineOnce();

        bool readNextLine();
    };
};