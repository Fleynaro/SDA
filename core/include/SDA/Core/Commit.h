#pragma once
#include "SDA/Core/Event/EventPipe.h"

namespace sda
{
    static const size_t CommitEventTopic = TopicName("CommitEventTopic");

    struct CommitBeginEvent : Event {
        CommitBeginEvent() : Event(CommitEventTopic) {}
    };

    struct CommitEndEvent : Event {
        CommitEndEvent() : Event(CommitEventTopic) {}
    };

    class CommitScope {
        std::shared_ptr<EventPipe> m_pipe;
    public:
        CommitScope(std::shared_ptr<EventPipe> pipe);

        ~CommitScope();
    };

    std::shared_ptr<EventPipe> CommitPipe();
};