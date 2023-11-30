#include "SDA/Core/Commit.h"
#include <memory>

using namespace sda;

CommitScope::CommitScope(std::shared_ptr<EventPipe> pipe)
    : m_pipe(pipe)
{
    m_pipe->send(CommitBeginEvent());
}

CommitScope::~CommitScope() {
    m_pipe->send(CommitEndEvent());
}

std::shared_ptr<EventPipe> sda::CommitPipe() {
    auto commitLevel = std::make_shared<size_t>(0);
    return EventPipe::New()
        ->process([commitLevel](const Event& event, const EventNext& next) {
            if (dynamic_cast<const CommitBeginEvent*>(&event)) {
                if (++*commitLevel == 1) {
                    next(event);
                }
            } else if (dynamic_cast<const CommitEndEvent*>(&event)) {
                if (--*commitLevel == 0) {
                    next(event);
                }
            }
        });
}

sda::OptimizedCommitPipeInfo sda::OptimizedCommitPipe(
    const std::function<void(const EventNext&)>& commitEndHandler,
    const EventFilter& filter)
{
    // All input events are moving through <pipeIn> and filtered:
    // - if event is emitted within some commit, it is taken by <commitPipeIn> and when commit ends it leaves through <pipeOut>
    // - if event is emitted outside of commit, it immediately leaves <pipeIn> through <pipeOut>
    struct Data {
        bool commit = false;
        bool locked = false;
    };
    auto data = std::make_shared<Data>();
    auto pipeIn = EventPipe::New("OptimizedCommitPipe::PipeIn");
    auto pipeOut = EventPipe::New("OptimizedCommitPipe::PipeOut");
    auto commitPipeIn = EventPipe::New("OptimizedCommitPipe::CommitPipeIn");
    auto optimizedPipe = EventPipe::Combine(pipeIn, pipeOut);
    auto commitPipe = pipeIn->connect(CommitPipe());
    commitPipe->subscribe(std::function([data](const CommitBeginEvent& event) {
        data->commit = true;
    }));
    commitPipe->subscribe(std::function([data, pipeOut, commitEndHandler](const CommitEndEvent& event) {
        data->commit = false;
        if (!data->locked) {
            data->locked = true;
            commitEndHandler([pipeOut](const Event& event) {
                // event leaves the optimized tube when commit ends (within commit)
                pipeOut->send(event);
            });
            data->locked = false;
        }
    }));
    pipeIn->subscribe(std::function([data, commitPipeIn, pipeOut, filter](const Event& event) {
        if (filter(event)) {
            if (data->commit && event.topic != CommitEventTopic) {
                commitPipeIn->send(event);
            } else {
                // event leaves the optimized tube immediately (no commit)
                pipeOut->send(event);
            }
        }
    }));
    return {
        optimizedPipe,
        commitPipeIn,
        pipeOut
    };
}
