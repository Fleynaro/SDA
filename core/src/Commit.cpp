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