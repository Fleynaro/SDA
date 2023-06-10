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
        EventPipe* m_pipe;
    public:
        CommitScope(EventPipe* pipe);

        ~CommitScope();
    };

    EventPipe CommitPipe();
};