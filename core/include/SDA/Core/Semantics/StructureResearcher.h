#pragma once
#include "DataFlowSemantics.h"
#include "ConstConditionSemantics.h"
#include "SDA/Core/Commit.h"
#include "SDA/Core/DataType/StructureDataType.h"
#include "SDA/Core/IRcode/IRcodeBlock.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/IRcode/IRcodeHelper.h"

namespace sda::semantics
{
    struct Structure {

    };

    class StructureRepository
    {
        
    public:

    };

    class StructureResearcher
    {
        ircode::Program* m_program;
        Platform* m_platform;
        StructureRepository* m_structureRepo;

        class EventHandler
        {
            StructureResearcher* m_researcher;

            void handleDataFlowNodePredecessorAdded(const DataFlowNodePredecessorAddedEvent& event) {
                
            }
        public:
            EventHandler(StructureResearcher* researcher) : m_researcher(researcher) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe->subscribeMethod(this, &EventHandler::handleDataFlowNodePredecessorAdded);
                return pipe;
            }
        };
        EventHandler m_ircodeEventHandler;
    public:
        StructureResearcher(
            ircode::Program* program,
            Platform* platform,
            StructureRepository* structureRepo
        )
            : m_program(program)
            , m_platform(platform)
            , m_structureRepo(structureRepo)
            , m_ircodeEventHandler(this)
        {
            m_program->getEventPipe()->connect(m_ircodeEventHandler.getEventPipe());
        }
    };
};
