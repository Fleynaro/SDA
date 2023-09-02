#pragma once
#include "PcodeFunctionGraph.h"
#include "PcodeStructurer.h"
#include "SDA/Core/Utils/AbstractPrinter.h"

namespace sda::pcode
{
    class Printer : public utils::AbstractPrinter
    {
        const RegisterRepository* m_regRepo;
        Graph* m_graph;
    public:
        static inline const Token MNEMONIC = PARENT;
        static inline const Token REGISTER = PARENT + 1;
        static inline const Token VIRT_REGISTER = PARENT + 2;

        Printer(const RegisterRepository* regRepo = nullptr, Graph* graph = nullptr);

        static std::string Print(const Instruction* instruction, const RegisterRepository* regRepo = nullptr);

        virtual void printFunctionGraph(FunctionGraph* functionGraph);

        virtual void printBlock(Block* block, size_t level = 0);

        virtual void printInstruction(const Instruction* instruction) const;

        virtual void printVarnode(std::shared_ptr<Varnode> varnode, bool printSizeAndOffset = true) const;

        StructTreePrinter::PrinterFunction getCodePrinter();

        StructTreePrinter::PrinterFunction getConditionPrinter();
    };
};