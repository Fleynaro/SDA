#pragma once
#include "SDA/Core/Event/Event.h"
#include "IRcodeProgram.h"

namespace sda::ircode
{
   static const size_t IRcodeEventTopic = TopicName("IRcodeEventTopic");

    // When a function is created
    struct FunctionCreatedEvent : Event {
        Function* function;

        FunctionCreatedEvent(Function* function)
            : Event(IRcodeEventTopic)
            , function(function)
        {}
    };

    // When a function is removed
    struct FunctionRemovedEvent : Event {
        Function* function;

        FunctionRemovedEvent(Function* function)
            : Event(IRcodeEventTopic)
            , function(function)
        {}
    };

    // When a function is decompiled
    struct FunctionDecompiledEvent : Event {
        Function* function;
        std::list<Block*> blocks;

        FunctionDecompiledEvent(Function* function, const std::list<Block*>& blocks)
            : Event(IRcodeEventTopic)
            , function(function)
            , blocks(blocks)
        {}
    };

    // When a function signature is changed
    struct FunctionSignatureChangedEvent : Event {
        Function* function;
        std::vector<std::shared_ptr<Variable>> m_oldParamVars;
        std::shared_ptr<Variable> m_oldReturnVar;

        FunctionSignatureChangedEvent(
            Function* function,
            const std::vector<std::shared_ptr<Variable>>& oldParamVars,
            const std::shared_ptr<Variable>& oldReturnVar
        )
            : Event(IRcodeEventTopic)
            , function(function)
            , m_oldParamVars(oldParamVars)
            , m_oldReturnVar(oldReturnVar)
        {}
    };

    // When a block is created
    struct BlockCreatedEvent : Event {
        Block* block;

        BlockCreatedEvent(Block* block)
            : Event(IRcodeEventTopic)
            , block(block)
        {}
    };

    // When a block is removed
    struct BlockRemovedEvent : Event {
        Block* block;

        BlockRemovedEvent(Block* block)
            : Event(IRcodeEventTopic)
            , block(block)
        {}
    };

    // When an operation is added
    struct OperationAddedEvent : Event {
        const Operation* op;

        OperationAddedEvent(const Operation* op)
            : Event(IRcodeEventTopic)
            , op(op)
        {}
    };

    // When an operation is removed
    struct OperationRemovedEvent : Event {
        const Operation* op;

        OperationRemovedEvent(const Operation* op)
            : Event(IRcodeEventTopic)
            , op(op)
        {}
    };
};