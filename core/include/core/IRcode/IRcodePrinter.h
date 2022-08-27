#pragma once
#include "IRcodeOperation.h"
#include "IRcodeDataTypeProvider.h"
#include "Core/Pcode/PcodePrinter.h"

namespace sda::ircode
{
    class Printer
    {
        pcode::Printer* m_pcodePrinter;
        size_t m_commentingCounter = 0;
        bool m_extendInfo = false;
        DataTypeProvider* m_dataTypeProvider;
    public:
        Printer(pcode::Printer* pcodePrinter);

        void setDataTypeProvider(DataTypeProvider* dataTypeProvider);

        virtual void printOperation(Operation* operation);

        virtual void printValue(const Value* value, bool extended = false) const;

        virtual void printLinearExpr(const LinearExpression* linearExpr) const;

        void commenting(bool toggle);

        void setExtendInfo(bool toggle);

    protected:
        enum class Token {
            Operation,
            Variable,
            Comment,
            Other
        };

        void printToken(const std::string& text, Token token) const;

        virtual void printTokenImpl(const std::string& text, Token token) const = 0;
    };

    class StreamPrinter : public Printer
    {
        std::ostream& m_output;
    public:
        StreamPrinter(std::ostream& output, pcode::Printer* pcodePrinter);

    protected:
        void printTokenImpl(const std::string& text, Token token) const override;
    };
};