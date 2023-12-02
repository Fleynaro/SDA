#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/IRcode/IRcodePrinter.h"
#include "SDA/Core/Commit.h"
#include "SDA/Core/Utils/Logger.h"

using namespace sda;
using namespace sda::ircode;

Program::Program(pcode::Graph* graph, SymbolTable* globalSymbolTable)
    : m_graph(graph)
    , m_globalSymbolTable(globalSymbolTable)
{
    IF_PLOG(plog::debug) {
        m_graph->getEventPipe()->subscribe(std::function([&](const FunctionCreatedEvent& event) {
            PLOG_DEBUG << "FunctionCreatedEvent: " << event.function->getFunctionGraph()->getName();
        }));
        m_graph->getEventPipe()->subscribe(std::function([&](const FunctionRemovedEvent& event) {
            PLOG_DEBUG << "FunctionRemovedEvent: " << event.function->getName();
        }));
        m_graph->getEventPipe()->subscribe(std::function([&](const FunctionDecompiledEvent& event) {
            PLOG_DEBUG << "FunctionDecompiledEvent: " << event.function->getName();
        }));
        m_graph->getEventPipe()->subscribe(std::function([&](const BlockCreatedEvent& event) {
            PLOG_DEBUG << "BlockCreatedEvent: " << event.block->getName();
        }));
        m_graph->getEventPipe()->subscribe(std::function([&](const BlockRemovedEvent& event) {
            PLOG_DEBUG << "BlockRemovedEvent: " << event.block->getName();
        }));
        m_graph->getEventPipe()->subscribe(std::function([&](const OperationAddedEvent& event) {
            pcode::Printer pcodePrinter(getPlatform()->getRegisterRepository().get());
            ircode::Printer ircodePrinter(&pcodePrinter);
            std::stringstream ss;
            ircodePrinter.setOutput(ss);
            ircodePrinter.printOperation(event.op);
            PLOG_DEBUG << "OperationAddedEvent: " << ss.str();
        }));
        m_graph->getEventPipe()->subscribe(std::function([&](const OperationRemovedEvent& event) {
            pcode::Printer pcodePrinter(getPlatform()->getRegisterRepository().get());
            ircode::Printer ircodePrinter(&pcodePrinter);
            std::stringstream ss;
            ircodePrinter.setOutput(ss);
            ircodePrinter.printOperation(event.op);
            PLOG_DEBUG << "OperationRemovedEvent: " << ss.str();
        }));
        auto commitPipe = m_graph->getEventPipe()->connect(CommitPipe());
        commitPipe->subscribe(std::function([&](const CommitBeginEvent& event) {
            PLOG_DEBUG << "CommitBeginEvent";
        }));
        commitPipe->subscribe(std::function([&](const CommitEndEvent& event) {
            PLOG_DEBUG << "CommitEndEvent";
        }));
    }
}

sda::Platform* Program::getPlatform() const {
    return m_graph->getPlatform();
}

std::shared_ptr<EventPipe> Program::getEventPipe() {
    return m_graph->getEventPipe();
}

pcode::Graph* Program::getGraph() {
    return m_graph;
}

SymbolTable* Program::getGlobalSymbolTable() {
    return m_globalSymbolTable;
}

std::map<pcode::FunctionGraph*, Function>& Program::getFunctions() {
    return m_functions;
}

Function* Program::toFunction(pcode::FunctionGraph* functionGraph) {
    auto it = m_functions.find(functionGraph);
    if (it == m_functions.end()) {
        return nullptr;
    }
    return &it->second;
}

Function* Program::getFunctionAt(pcode::InstructionOffset offset) {
    auto functionGraph = m_graph->getFunctionGraphAt(offset);
    if (functionGraph) {
        return toFunction(functionGraph);
    }
    return nullptr;
}

std::list<Function*> Program::getFunctionsByCallInstruction(const pcode::Instruction* instr) {
    auto offset = GetTargetOffset(instr);
    if (offset != InvalidOffset) {
        auto function = getFunctionAt(offset);
        if (function) {
            return { function };
        }
        return {};
    }
    // TODO: virtual calls
    return {};
}

std::list<CallOperation*> Program::getCallsRefToFunction(Function* function) {
    std::list<CallOperation*> result;
    auto funcGraph = function->getFunctionGraph();
    auto fromFuncGraphs = funcGraph->getReferencesTo();
    for (auto fromFuncGraph : fromFuncGraphs) {
        for (auto& [offset, toFuncGraph] : fromFuncGraph->getReferencesFrom()) {
            if (toFuncGraph == funcGraph) {
                auto fromFunction = toFunction(fromFuncGraph);
                if (fromFunction) {
                    auto fromPcodeBlock = m_graph->getBlockAt(offset);
                    if (fromPcodeBlock) {
                        auto fromBlock = fromFunction->toBlock(fromPcodeBlock);
                        if (fromBlock) {
                            auto op = fromBlock->getOperationAt(offset);
                            if (auto callOp = dynamic_cast<ircode::CallOperation*>(op)) {
                                result.push_back(callOp);
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}
