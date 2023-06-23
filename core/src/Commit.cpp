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

std::shared_ptr<EventPipe> sda::OptimizedCommitPipe(
        const EventFilter& filter,
        std::shared_ptr<EventPipe>& commitPipeIn,
        const std::function<void(const EventNext&)>& commitEmitter)
{
    struct Data {
        bool commit = false;
        bool locked = false;
    };
    auto data = std::make_shared<Data>();
    auto pipeIn = EventPipe::New("OptimizedCommitPipe::PipeIn")
        ->filter(
            EventPipe::FilterOr(
                EventPipe::FilterTopic(CommitEventTopic),
                filter));
    pipeIn
        ->connect(CommitPipe())
        ->subscribe(std::function([data](const CommitBeginEvent& event) {
            data->commit = true;
        }));
    commitPipeIn = EventPipe::New("OptimizedCommitPipe::CommitPipeIn");
    auto commitPipeOut = commitPipeIn
        ->connect(CommitPipe())
        ->process(std::function([data, commitEmitter](const Event& event, const EventNext& next) {
            if (dynamic_cast<const CommitEndEvent*>(&event)) {
                data->commit = false;
                if (!data->locked) {
                    data->locked = true;
                    commitEmitter(next);
                    data->locked = false;
                }
            }
        }));
    // All input events are moving through pipeIn and filtered:
    // - if event is emitted within some commit, it is handled by commitPipeIn and when commit ends it leaves through commitPipeOut
    // - if event is emitted outside of commit, it immediately leaves pipeIn
    return EventPipe::Combine(
        pipeIn,
        pipeIn->connect(
            EventPipe::If(
                std::function([data](const Event& event) {
                    return data->commit;
                }),
                EventPipe::Combine(commitPipeIn, commitPipeOut),
                EventPipe::New("OptimizedCommitPipe::PipeOut")
            )
        )
    );
}