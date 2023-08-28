#pragma once
#include "IRcodeFunction.h"
#include "IRcodeDataTypeProvider.h"
#include "SDA/Core/Pcode/PcodePrinter.h"

namespace sda::ircode
{
    class Printer : public utils::AbstractPrinter
    {
        pcode::Printer* m_pcodePrinter;
        DataTypeProvider* m_dataTypeProvider;
        using OperationCommentProvider = std::function<std::string(const Operation*)>;
        OperationCommentProvider m_operationCommentProvider;
    public:
        bool m_printVarAddressAlways = false;
        static inline const Token OPERATION = KEYWORD;
        static inline const Token VARIABLE = IDENTIFIER;

        Printer(pcode::Printer* pcodePrinter);

        void setOperationCommentProvider(const OperationCommentProvider& operationCommentProvider);

        virtual void printFunction(Function* function);

        virtual void printBlock(Block* block, size_t level = 0);

        virtual void printOperation(const Operation* operation);

        virtual void printValue(std::shared_ptr<Value> value, bool extended = false);

        virtual void printLinearExpr(const LinearExpression* linearExpr);

        pcode::StructTreePrinter::PrinterFunction getCodePrinter(Function* function);

        pcode::StructTreePrinter::PrinterFunction getConditionPrinter(Function* function);
    };
};