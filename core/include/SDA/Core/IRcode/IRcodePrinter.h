#pragma once
#include "IRcodeOperation.h"
#include "IRcodeDataTypeProvider.h"
#include "SDA/Core/Pcode/PcodePrinter.h"

namespace sda::ircode
{
    class Printer : public utils::AbstractPrinter
    {
        pcode::Printer* m_pcodePrinter;
        bool m_extendInfo = false;
        DataTypeProvider* m_dataTypeProvider;
    public:
        static inline const Token OPERATION = KEYWORD;
        static inline const Token VARIABLE = IDENTIFIER;

        Printer(pcode::Printer* pcodePrinter);

        void setDataTypeProvider(DataTypeProvider* dataTypeProvider);

        void setExtendInfo(bool toggle);

        virtual void printOperation(Operation* operation);

        virtual void printValue(const Value* value, bool extended = false) const;

        virtual void printLinearExpr(const LinearExpression* linearExpr) const;
    };
};