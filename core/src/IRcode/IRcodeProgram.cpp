#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/IRcode/IRcodePrinter.h"
#include "SDA/Core/Commit.h"
#include "SDA/Core/Utils/Logger.h"

using namespace sda;
using namespace sda::ircode;

std::shared_ptr<EventPipe> CreateOptimizedUpdateBlocksEventPipe(Program* program) {
    struct Data {
        std::map<pcode::FunctionGraph*, std::set<pcode::Block*>> pcodeBlocksToUpdate;
    };
    auto data = std::make_shared<Data>();
    auto filter = std::function([](const Event& event) {
        auto e =  dynamic_cast<const pcode::BlockUpdatedEvent*>(&event);
        return dynamic_cast<const pcode::FunctionGraphRemovedEvent*>(&event) ||
                (e && e->requested);
    });
    auto commitEmitter = std::function([data, program](const EventNext& next) {
        while (!data->pcodeBlocksToUpdate.empty()) {
            auto it = data->pcodeBlocksToUpdate.begin(); // TODO: optimize by firstly selecting the lowest function
            auto& [funcGraph, pcodeBlocks] = *it;
            auto function = program->toFunction(funcGraph);
            utils::BitSet mutualDomBlockSet;
            for (auto pcodeBlock : pcodeBlocks) {
                if (pcodeBlock == *pcodeBlocks.begin()) {
                    mutualDomBlockSet = pcodeBlock->getDominantBlocksSet();
                } else {
                    mutualDomBlockSet = mutualDomBlockSet & pcodeBlock->getDominantBlocksSet();
                }
            }
            auto mutualDomBlocks = funcGraph->toBlocks(mutualDomBlockSet);
            pcode::Block* mostDominatedBlock = nullptr;
            size_t maxSize = 0;
            for (auto pcodeBlock : mutualDomBlocks) {
                auto size = pcodeBlock->getDominantBlocks().size();
                if (size > maxSize) {
                    mostDominatedBlock = pcodeBlock;
                    maxSize = size;
                }
            }
            assert(mostDominatedBlock);
            data->pcodeBlocksToUpdate.erase(it);
            next(pcode::BlockUpdatedEvent(mostDominatedBlock));
        }
    });
    std::shared_ptr<EventPipe> commitPipeIn;
    auto result = OptimizedCommitPipe(filter, commitPipeIn, commitEmitter);
    commitPipeIn->subscribe(std::function([data](const pcode::BlockUpdatedEvent& event) {
        data->pcodeBlocksToUpdate[event.block->getFunctionGraph()].insert(event.block);
    }));
    commitPipeIn->subscribe(std::function([data](const pcode::FunctionGraphRemovedEvent& event) {
        data->pcodeBlocksToUpdate.erase(event.functionGraph);
    }));
    return result;
}

void Program::PcodeEventHandler::handleBlockUpdatedEvent(const pcode::BlockUpdatedEvent& event) {
    auto function = m_program->toFunction(event.block->getFunctionGraph());
    auto block = function->toBlock(event.block);
    block->update();
}

void Program::PcodeEventHandler::handleBlockFunctionGraphChanged(const pcode::BlockFunctionGraphChangedEvent& event) {
    if (event.oldFunctionGraph) {
        auto oldFunction = m_program->toFunction(event.oldFunctionGraph);
        auto block = oldFunction->toBlock(event.block);
        m_program->getEventPipe()->send(BlockRemovedEvent(block));
        oldFunction->getBlocks().erase(event.block);
    }
    auto newFunction = m_program->toFunction(event.newFunctionGraph);
    newFunction->getBlocks().emplace(event.block, Block(event.block, newFunction));
    m_program->getEventPipe()->send(BlockCreatedEvent(
        newFunction->toBlock(event.block)));
}

void Program::PcodeEventHandler::handleFunctionGraphCreated(const pcode::FunctionGraphCreatedEvent& event) {
    m_program->m_functions.emplace(event.functionGraph, Function(m_program, event.functionGraph));
    m_program->getEventPipe()->send(FunctionCreatedEvent(
        m_program->toFunction(event.functionGraph)));
}

void Program::PcodeEventHandler::handleFunctionGraphRemoved(const pcode::FunctionGraphRemovedEvent& event) {
    m_program->getEventPipe()->send(FunctionRemovedEvent(
        m_program->toFunction(event.functionGraph)));
    m_program->m_functions.erase(event.functionGraph);
}

std::shared_ptr<EventPipe> Program::PcodeEventHandler::getEventPipe() {
    auto pipe = EventPipe::New("Program::PcodeEventHandler");
    pipe
        ->connect(CreateOptimizedUpdateBlocksEventPipe(m_program))
        ->subscribeMethod(this, &PcodeEventHandler::handleBlockUpdatedEvent);
    pipe->subscribeMethod(this, &PcodeEventHandler::handleBlockFunctionGraphChanged);
    pipe->subscribeMethod(this, &PcodeEventHandler::handleFunctionGraphCreated);
    pipe->subscribeMethod(this, &PcodeEventHandler::handleFunctionGraphRemoved);
    return pipe;
}

void Program::ContextEventHandler::handleObjectModified(const ObjectModifiedEvent& event) {
    if (auto signatureDt = dynamic_cast<SignatureDataType*>(event.object)) {
        for (auto funcSymbol : signatureDt->getFunctionSymbols()) {
            if (auto symbolTable = funcSymbol->getSymbolTable()) {
                if (symbolTable == m_program->m_globalSymbolTable) {
                    auto function = m_program->getFunctionAt(funcSymbol->getOffset());
                    auto blocks = m_program->getBlocksRefToFunction(function);
                    CommitScope commitScope(m_program->getEventPipe());
                    for (auto block : blocks) {
                        m_program->getEventPipe()->send(pcode::BlockUpdatedEvent(block->getPcodeBlock()));
                    }
                }
            }
        }
    }
}

std::shared_ptr<EventPipe> Program::ContextEventHandler::getEventPipe() {
    auto pipe = Context::CreateOptimizedEventPipe();
    pipe->subscribeMethod(this, &ContextEventHandler::handleObjectModified);
    return pipe;
}

Program::Program(pcode::Graph* graph, SymbolTable* globalSymbolTable)
    : m_graph(graph)
    , m_globalSymbolTable(globalSymbolTable)
    , m_pcodeEventHandler(this)
    , m_contextEventHandler(this)
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
            auto platform = m_globalSymbolTable->getContext()->getPlatform();
            pcode::Printer pcodePrinter(platform->getRegisterRepository().get());
            ircode::Printer ircodePrinter(&pcodePrinter);
            std::stringstream ss;
            ircodePrinter.setOutput(ss);
            ircodePrinter.printOperation(event.op);
            PLOG_DEBUG << "OperationAddedEvent: " << ss.str();
        }));
        m_graph->getEventPipe()->subscribe(std::function([&](const OperationRemovedEvent& event) {
            auto platform = m_globalSymbolTable->getContext()->getPlatform();
            pcode::Printer pcodePrinter(platform->getRegisterRepository().get());
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
    m_graph->getEventPipe()->connect(m_contextEventHandler.getEventPipe());
    m_graph->getEventPipe()->connect(m_pcodeEventHandler.getEventPipe());
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

std::list<Block*> Program::getBlocksRefToFunction(Function* function) {
    std::list<Block*> result;
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
                            result.push_back(fromBlock);
                        }
                    }
                }
            }
        }
    }
    return result;
}
