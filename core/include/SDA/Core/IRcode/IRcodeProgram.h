#pragma once
#include "IRcodeFunction.h"
#include "SDA/Core/Pcode/PcodeGraph.h"
#include "SDA/Core/Pcode/PcodeEvents.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/ContextEvents.h"

namespace sda::ircode
{
    class Program
    {
        friend class ContextSync;
        friend class PcodeSync;
        pcode::Graph* m_graph;
        SymbolTable* m_globalSymbolTable;
        std::map<pcode::FunctionGraph*, Function> m_functions;
    public:
        Program(pcode::Graph* graph, SymbolTable* globalSymbolTable);

        std::shared_ptr<EventPipe> getEventPipe();

        pcode::Graph* getGraph();

        SymbolTable* getGlobalSymbolTable();

        std::map<pcode::FunctionGraph*, Function>& getFunctions();

        Function* toFunction(pcode::FunctionGraph* functionGraph);

        Function* getFunctionAt(pcode::InstructionOffset offset);

        std::list<Function*> getFunctionsByCallInstruction(const pcode::Instruction* instr);

        std::list<CallOperation*> getCallsRefToFunction(Function* function);
    };
};