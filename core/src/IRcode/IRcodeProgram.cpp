#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/Commit.h"

using namespace sda;
using namespace sda::ircode;

void UpdateBlocks(Program* program, const std::set<pcode::Block*>& pcodeBlocks) {
    std::map<pcode::FunctionGraph*, std::list<pcode::Block*>> blocksToUpdateByFunction;
    for (auto pcodeBlock : pcodeBlocks) {
        auto pcodeFunctionGraph = pcodeBlock->getFunctionGraph();
        blocksToUpdateByFunction[pcodeFunctionGraph].push_back(pcodeBlock);
    }
    for (auto& [pcodeFunctionGraph, pcodeBlocksToUpdate] : blocksToUpdateByFunction) {
        auto function = program->toFunction(pcodeFunctionGraph);
        utils::BitSet mutualDomBlockSet;
        for (auto pcodeBlock : pcodeBlocksToUpdate) {
            if (pcodeBlock == *pcodeBlocksToUpdate.begin()) {
                mutualDomBlockSet = pcodeBlock->getDominantBlocksSet();
            } else {
                mutualDomBlockSet = mutualDomBlockSet & pcodeBlock->getDominantBlocksSet();
            }
        }
        auto mutualDomBlocks = pcodeFunctionGraph->toBlocks(mutualDomBlockSet);
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
        auto block = function->toBlock(mostDominatedBlock);
        block->update();
    }
}

std::shared_ptr<EventPipe> UpdateBlocksEventPipe(Program* program) {
    struct Data {
        bool commit = false;
        std::set<pcode::Block*> pcodeBlocksToUpdate;
    };
    auto data = std::make_shared<Data>();
    auto pipe = EventPipe::New("UpdateBlocksEventPipe");
    auto commitPipe = pipe->connect(CommitPipe());
    commitPipe->subscribe(std::function([data](const CommitBeginEvent& event) {
        data->commit = true;
    }));
    commitPipe->subscribe(std::function([program, data](const CommitEndEvent& event) {
        auto blocks = std::move(data->pcodeBlocksToUpdate);
        data->pcodeBlocksToUpdate.clear();
        data->commit = false;
        // call UpdateBlocks in the end to avoid endless recursion
        UpdateBlocks(program, blocks);
    }));
    pipe->subscribe(std::function([program, data](const pcode::BlockUpdateRequestedEvent& event) {
        if (data->commit) {
            data->pcodeBlocksToUpdate.insert(event.block);
        } else {
            UpdateBlocks(program, { event.block });
        }
    }));
    return pipe;
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
    pipe->connect(UpdateBlocksEventPipe(m_program));
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
                    std::set<pcode::Block*> pcodeBlocks;
                    for (auto block : blocks) {
                        pcodeBlocks.insert(block->getPcodeBlock());
                    }
                    UpdateBlocks(m_program, pcodeBlocks);
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
    m_graph->getEventPipe()->connect(m_pcodeEventHandler.getEventPipe());
    m_graph->getEventPipe()->connect(m_contextEventHandler.getEventPipe());
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
