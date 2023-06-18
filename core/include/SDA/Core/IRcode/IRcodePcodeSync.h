#pragma once
#include "IRcodeProgram.h"
#include "SDA/Core/Pcode/PcodeEvents.h"

namespace sda::ircode
{
    class PcodeSync
    {
        Program* m_program;

        void handleBlockUpdatedEvent(const pcode::BlockUpdatedEvent& event);

        void handleBlockFunctionGraphChanged(const pcode::BlockFunctionGraphChangedEvent& event);

        void handleFunctionGraphCreated(const pcode::FunctionGraphCreatedEvent& event);

        void handleFunctionGraphRemoved(const pcode::FunctionGraphRemovedEvent& event);
    public:
        PcodeSync(Program* program);

        std::shared_ptr<EventPipe> getEventPipe();
    };
};