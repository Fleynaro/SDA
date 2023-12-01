#pragma once
#include <string>
#include <sstream>

namespace utils
{
    class AbstractPrinter
    {
        std::ostream* m_output = nullptr;
        size_t m_tabSize = 4;
        size_t m_blockCounter = 0;
        size_t m_commentingCounter = 0;
        AbstractPrinter* m_parentPrinter = nullptr;
    public:
        using Token = size_t;
        static inline const Token SYMBOL = 1;
        static inline const Token SPEC_SYMBOL = 2;
        static inline const Token KEYWORD = 3;
        static inline const Token IDENTIFIER = 4;
        static inline const Token NUMBER = 5;
        static inline const Token STRING = 6;
        static inline const Token COMMENT = 7;
        static inline const Token PARENT = 100;

        AbstractPrinter() = default;

        void setOutput(std::ostream& output);

        void setTabSize(size_t tabSize);

        void setParentPrinter(AbstractPrinter* parent);

        virtual void startBlock();

        virtual void endBlock();

        virtual void startCommenting();

        virtual void endCommenting();

        void newLine() const;

        void newTabs() const;
        
        void printToken(const std::string& text, Token token) const;

        std::ostream& out() const;

    protected:
        void printComment(const std::string& text) const;

        virtual void printTokenImpl(const std::string& text, Token token) const;
    };
};