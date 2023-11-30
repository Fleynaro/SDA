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

    struct OptimizedCommitPipeInfo {
        std::shared_ptr<EventPipe> optimizedPipe;
        std::shared_ptr<EventPipe> commitPipeIn;
        std::shared_ptr<EventPipe> pipeOut;
    };
    // Optimizes event stream by joining multiple same events into one event within a commit to reduce the number of event handler calls
    OptimizedCommitPipeInfo OptimizedCommitPipe(
        const std::function<void(const EventNext&)>& commitEndHandler,
        const EventFilter& filter = EventPipe::FilterTrue);
};