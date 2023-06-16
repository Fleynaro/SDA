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

    // Allows to properly subscribe to the CommitBeginEvent/CommitEndEvent events
    std::shared_ptr<EventPipe> CommitPipe();

    // Joins multiple same events into one event within a commit to reduce the number of event handler calls
    std::shared_ptr<EventPipe> OptimizedCommitPipe(
        const EventFilter& filter,
        std::shared_ptr<EventPipe>& commitPipeIn,
        const std::function<void(const EventNext&)>& commitEmitter);
};