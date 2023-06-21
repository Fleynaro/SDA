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

        class PcodeEventHandler
        {
            Program* m_program;

            void handleBlockUpdatedEvent(const pcode::BlockUpdatedEvent& event);

            void handleBlockFunctionGraphChanged(const pcode::BlockFunctionGraphChangedEvent& event);

            void handleFunctionGraphCreated(const pcode::FunctionGraphCreatedEvent& event);

            void handleFunctionGraphRemoved(const pcode::FunctionGraphRemovedEvent& event);

        public:
            PcodeEventHandler(Program* program) : m_program(program) {}

            std::shared_ptr<EventPipe> getEventPipe();
        };

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