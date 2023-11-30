#include "SDA/Core/IRcode/IRcodePcodeSync.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/Commit.h"

using namespace sda;
using namespace sda::ircode;

// Allows to optimize event handling by avoiding multiple same events (BlockUpdatedEvent) within commit
std::shared_ptr<EventPipe> CreateOptimizedUpdateBlocksEventPipe(Program* program) {
    struct Data {
        using Map = std::map<pcode::FunctionGraph*, std::set<pcode::Block*>>;
        Map pcodeBlocksToUpdate;

        Map::iterator selectLowestFunction() {
            // TODO: optimize by firstly selecting the lowest function
            Map::iterator resultIt = pcodeBlocksToUpdate.begin();
            for (auto it = pcodeBlocksToUpdate.begin(); it != pcodeBlocksToUpdate.end(); ++it) {
                if (it->first->getEntryOffset() < resultIt->first->getEntryOffset()) {
                    resultIt = it;
                }
            }
            return resultIt;
        }
    };
    auto data = std::make_shared<Data>();
    auto commitEndHandler = std::function([data, program](const EventNext& next) {
        while (!data->pcodeBlocksToUpdate.empty()) {
            auto it = data->selectLowestFunction();
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
    auto [optPipe, commitPipeIn, _] = OptimizedCommitPipe(commitEndHandler);
    commitPipeIn->subscribe(std::function([data](const pcode::BlockUpdatedEvent& event) {
        if (!event.requested) return;
        data->pcodeBlocksToUpdate[event.block->getFunctionGraph()].insert(event.block);
    }));
    commitPipeIn->subscribe(std::function([data](const pcode::BlockRemovedEvent& event) {
        data->pcodeBlocksToUpdate[event.block->getFunctionGraph()].erase(event.block);
    }));
    commitPipeIn->subscribe(std::function([data](const pcode::FunctionGraphRemovedEvent& event) {
        data->pcodeBlocksToUpdate.erase(event.functionGraph);
    }));
    commitPipeIn->subscribe(std::function([data](const pcode::BlockFunctionGraphChangedEvent& event) {
        if (event.oldFunctionGraph) {
            auto it = data->pcodeBlocksToUpdate.find(event.oldFunctionGraph);
            if (it != data->pcodeBlocksToUpdate.end()) {
                it->second.erase(event.block);
                if (event.newFunctionGraph) {
                    data->pcodeBlocksToUpdate[event.newFunctionGraph].insert(event.block);
                }
            }
        }
    }));
    return optPipe;
}

PcodeSync::PcodeSync(Program* program)
    : m_program(program)
{}

void PcodeSync::handleBlockUpdatedEvent(const pcode::BlockUpdatedEvent& event) {
    auto function = m_program->toFunction(event.block->getFunctionGraph());
    auto block = function->toBlock(event.block);
    block->update();
}

void PcodeSync::handleBlockFunctionGraphChanged(const pcode::BlockFunctionGraphChangedEvent& event) {
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

void PcodeSync::handleFunctionGraphCreated(const pcode::FunctionGraphCreatedEvent& event) {
    m_program->m_functions.emplace(event.functionGraph, Function(m_program, event.functionGraph));
    m_program->getEventPipe()->send(FunctionCreatedEvent(
        m_program->toFunction(event.functionGraph)));
}

void PcodeSync::handleFunctionGraphRemoved(const pcode::FunctionGraphRemovedEvent& event) {
    m_program->getEventPipe()->send(FunctionRemovedEvent(
        m_program->toFunction(event.functionGraph)));
    m_program->m_functions.erase(event.functionGraph);
}

std::shared_ptr<EventPipe> PcodeSync::getEventPipe() {
    auto pipe = EventPipe::New("PcodeSync");
    pipe
        ->connect(CreateOptimizedUpdateBlocksEventPipe(m_program))
        ->subscribeMethod(this, &PcodeSync::handleBlockUpdatedEvent);
    pipe->subscribeMethod(this, &PcodeSync::handleBlockFunctionGraphChanged);
    pipe->subscribeMethod(this, &PcodeSync::handleFunctionGraphCreated);
    pipe->subscribeMethod(this, &PcodeSync::handleFunctionGraphRemoved);
    return pipe;
}
