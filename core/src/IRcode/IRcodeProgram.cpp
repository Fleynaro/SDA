#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/Pcode/PcodeGraph.h"

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

void Program::PcodeGraphCallbacks::onBlockUpdateRequestedImpl(pcode::Block* pcodeBlock) {
    m_pcodeBlocksToUpdate.insert(pcodeBlock);
}

void Program::PcodeGraphCallbacks::onBlockFunctionGraphChangedImpl(pcode::Block* pcodeBlock, pcode::FunctionGraph* oldFunctionGraph, pcode::FunctionGraph* newFunctionGraph) {
    if (oldFunctionGraph) {
        auto oldFunction = m_program->toFunction(oldFunctionGraph);
        auto block = oldFunction->toBlock(pcodeBlock);
        m_program->getCallbacks()->onBlockRemoved(block);
        oldFunction->getBlocks().erase(pcodeBlock);
    }
    auto newFunction = m_program->toFunction(newFunctionGraph);
    newFunction->getBlocks().emplace(pcodeBlock, Block(pcodeBlock, newFunction));
    m_program->getCallbacks()->onBlockCreated(newFunction->toBlock(pcodeBlock));
}

void Program::PcodeGraphCallbacks::onFunctionGraphCreatedImpl(pcode::FunctionGraph* functionGraph) {
    m_program->m_functions.emplace(functionGraph, Function(m_program, functionGraph));
    m_program->getCallbacks()->onFunctionCreated(m_program->toFunction(functionGraph));
}

void Program::PcodeGraphCallbacks::onFunctionGraphRemovedImpl(pcode::FunctionGraph* functionGraph) {
    m_program->getCallbacks()->onFunctionRemoved(m_program->toFunction(functionGraph));
    m_program->m_functions.erase(functionGraph);
}

void Program::PcodeGraphCallbacks::onCommitStartedImpl() {
    m_commitStarted = true;
}

void Program::PcodeGraphCallbacks::onCommitEndedImpl() {
    m_commitStarted = false;
    UpdateBlocks(m_program, m_pcodeBlocksToUpdate);
    m_pcodeBlocksToUpdate.clear();
}

void Program::ContextCallbacks::onObjectModifiedImpl(Object* object) {
    if (auto signatureDt = dynamic_cast<SignatureDataType*>(object)) {
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

Program::Program(pcode::Graph* graph, SymbolTable* globalSymbolTable)
    : m_graph(graph)
    , m_globalSymbolTable(globalSymbolTable)
    , m_pcodeGraphCallbacks(std::make_shared<PcodeGraphCallbacks>(this))
    , m_contextCallbacks(std::make_shared<ContextCallbacks>(this))
    , m_callbacks(std::make_shared<Callbacks>())
{
    {
        // graph callbacks
        auto prevCallbacks = m_graph->getCallbacks();
        m_pcodeGraphCallbacks->setPrevCallbacks(prevCallbacks);
        m_graph->setCallbacks(m_pcodeGraphCallbacks);
    }
    {
        // context callbacks
        auto context = m_globalSymbolTable->getContext();
        auto prevCallbacks = context->getCallbacks();
        m_contextCallbacks->setPrevCallbacks(prevCallbacks);
        context->setCallbacks(m_contextCallbacks);
    }
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

void Program::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<Program::Callbacks> Program::getCallbacks() const {
    return m_callbacks;
}
