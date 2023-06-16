#pragma once
#include "SDA/Core/Event/Event.h"
#include "PcodeFunctionGraph.h"

namespace sda::pcode
{
    static const size_t PcodeEventTopic = TopicName("PcodeEventTopic");

    // When an instruction is added to the graph
    struct InstructionAddedEvent : Event {
        const Instruction* instruction;
        InstructionOffset nextOffset;

        InstructionAddedEvent(const Instruction* instruction, InstructionOffset nextOffset)
            : Event(PcodeEventTopic)
            , instruction(instruction)
            , nextOffset(nextOffset)
        {}
    };

    // When an instruction is removed from the graph
    struct InstructionRemovedEvent : Event {
        const Instruction* instruction;

        InstructionRemovedEvent(const Instruction* instruction)
            : Event(PcodeEventTopic)
            , instruction(instruction)
        {}
    };

    // When a block is created
    struct BlockCreatedEvent : Event {
        Block* block;

        BlockCreatedEvent(Block* block)
            : Event(PcodeEventTopic)
            , block(block)
        {}
    };

    // When a block is updated
    struct BlockUpdatedEvent : Event {
        Block* block;
        bool requested;

        BlockUpdatedEvent(Block* block, bool requested = true)
            : Event(PcodeEventTopic)
            , block(block)
            , requested(requested)
        {}
    };

    // When a block's function graph is changed
    struct BlockFunctionGraphChangedEvent : Event {
        Block* block;
        FunctionGraph* oldFunctionGraph;
        FunctionGraph* newFunctionGraph;

        BlockFunctionGraphChangedEvent(Block* block, FunctionGraph* oldFunctionGraph, FunctionGraph* newFunctionGraph)
            : Event(PcodeEventTopic)
            , block(block)
            , oldFunctionGraph(oldFunctionGraph)
            , newFunctionGraph(newFunctionGraph)
        {}
    };

    // When a block is removed
    struct BlockRemovedEvent : Event {
        Block* block;

        BlockRemovedEvent(Block* block)
            : Event(PcodeEventTopic)
            , block(block)
        {}
    };

    // When a function graph is created
    struct FunctionGraphCreatedEvent : Event {
        FunctionGraph* functionGraph;

        FunctionGraphCreatedEvent(FunctionGraph* functionGraph)
            : Event(PcodeEventTopic)
            , functionGraph(functionGraph)
        {}
    };

    // When a function graph is removed
    struct FunctionGraphRemovedEvent : Event {
        FunctionGraph* functionGraph;

        FunctionGraphRemovedEvent(FunctionGraph* functionGraph)
            : Event(PcodeEventTopic)
            , functionGraph(functionGraph)
        {}
    };

    // When an unvisited offset is found
    struct UnvisitedOffsetFoundEvent : Event {
        InstructionOffset offset;

        UnvisitedOffsetFoundEvent(InstructionOffset offset)
            : Event(PcodeEventTopic)
            , offset(offset)
        {}
    };
};
