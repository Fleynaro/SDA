#pragma once
#include "PcodeFunctionGraph.h"
#include "SDA/Core/Utils/AbstractPrinter.h"

namespace sda::pcode
{
    class Printer : public utils::AbstractPrinter
    {
        const RegisterRepository* m_regRepo;
        Graph* m_graph;
    public:
        static inline const Token MNEMONIC = KEYWORD;
        static inline const Token REGISTER = IDENTIFIER;
        static inline const Token VIRT_REGISTER = IDENTIFIER;

        Printer(const RegisterRepository* regRepo, Graph* graph = nullptr);

        static std::string Print(const Instruction* instruction, const RegisterRepository* regRepo);

        virtual void printFunctionGraph(FunctionGraph* functionGraph);

        virtual void printBlock(Block* block, size_t level = 0);

        virtual void printInstruction(const Instruction* instruction) const;

        virtual void printVarnode(std::shared_ptr<Varnode> varnode, bool printSizeAndOffset = true) const;
    };
};