#pragma once
#include "IRcodeProgram.h"
#include "IRcodeEvents.h"
#include "SDA/Core/Platform/CallingConvention.h"

namespace sda::ircode
{
    class ContextSync
    {
        Program* m_program;
        SymbolTable* m_globalSymbolTable;
        std::shared_ptr<CallingConvention> m_callingConvention;

        void handleFunctionCreated(const FunctionCreatedEvent& event);

        void handleFunctionRemoved(const FunctionRemovedEvent& event);

        void handleObjectModified(const ObjectModifiedEvent& event);
    public:
        ContextSync(
            Program* program,
            SymbolTable* globalSymbolTable,
            std::shared_ptr<CallingConvention> callingConvention);

        std::shared_ptr<EventPipe> getEventPipe();
    };
};