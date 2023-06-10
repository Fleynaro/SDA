#pragma once
#include "IRcodeProgram.h"
#include "IRcodeEvents.h"
#include "SDA/Core/Platform/CallingConvention.h"

namespace sda::ircode
{
    class ContextSync
    {
        SymbolTable* m_globalSymbolTable;
        std::shared_ptr<CallingConvention> m_callingConvention;

        void handleFunctionCreated(const FunctionCreatedEvent& event);

        void handleFunctionRemoved(const FunctionRemovedEvent& event);
    public:
        ContextSync(
            SymbolTable* globalSymbolTable,
            std::shared_ptr<CallingConvention> callingConvention);

        EventPipe getEventPipe();
    };
};