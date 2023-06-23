#include "SDA/Core/Event/EventPipe.h"
#include <stack>

using namespace sda;

EventPipe::EventPipe(const std::string& name, const EventProcessor& processor)
    : m_name(name), m_processor(processor)
{}

void EventPipe::send(const Event& event) {
    m_processor(event, [this](const Event& event) {
        for (auto& handler : m_handlers)
            handler(event);
        for (auto& pipe : m_nextPipes)
            pipe->send(event);
    });
}

std::shared_ptr<EventPipe> EventPipe::process(const EventProcessor& processor) {
    return connect(std::make_shared<EventPipe>("Processor", processor));
}

std::shared_ptr<EventPipe> EventPipe::filter(const EventFilter& filter) {
    return connect(std::make_shared<EventPipe>("Filter", [filter](const Event& event, const EventNext& next) {
        if (filter(event))
            next(event);
    }));
}

EventUnsubscribe EventPipe::subscribe(const EventHandler& handler) {
    m_handlers.emplace_back(handler);
    auto handlerPtr = &m_handlers.back();
    return [this, handlerPtr]() {
        m_handlers.remove_if([handlerPtr](const EventHandler& handler) {
            return &handler == handlerPtr;
        });
    };
}

std::shared_ptr<EventPipe> EventPipe::connect(std::shared_ptr<EventPipe> pipe) {
    m_nextPipes.emplace_back(pipe);
    return m_nextPipes.back();
}

void EventPipe::disconnect(std::shared_ptr<EventPipe> pipe) {
    m_nextPipes.remove(pipe);
}

std::shared_ptr<EventPipe> EventPipe::New(const std::string& name) {
    return std::make_shared<EventPipe>(name, [](const Event& event, const EventNext& next) {
        next(event);
    });
}

std::shared_ptr<EventPipe> EventPipe::Combine(std::shared_ptr<EventPipe> pipeIn, std::shared_ptr<EventPipe> pipeOut) {
    // we need lock for nested case (see CommitTest.aggregateEventsNestedNotIgnored)
    auto locked = std::make_shared<bool>();
    return std::make_shared<EventPipe>("Combine", [pipeIn, pipeOut, locked](const Event& event, const EventNext& next) {
        if (!*locked) {
            *locked = true;
            auto unsubscribe = pipeOut->subscribe(next);
            pipeIn->send(event);
            unsubscribe();
            *locked = false;
        } else {
            pipeIn->send(event);
        }
    });
}

std::shared_ptr<EventPipe> EventPipe::If(
    const EventFilter& condition,
    std::shared_ptr<EventPipe> pipeThen,
    std::shared_ptr<EventPipe> pipeElse)
{
    struct Lock {
        bool then_;
        bool else_;
    };
    auto locked = std::make_shared<Lock>();
    return std::make_shared<EventPipe>("If", [condition, pipeThen, pipeElse, locked](const Event& event, const EventNext& next) {
        if (condition(event)) {
            if (!locked->then_) {
                locked->then_ = true;
                auto unsubscribe = pipeThen->subscribe(next);
                pipeThen->send(event);
                unsubscribe();
                locked->then_ = false;
            } else {
                pipeThen->send(event);
            }
        } else {
            if (!locked->else_) {
                locked->else_ = true;
                auto unsubscribe = pipeElse->subscribe(next);
                pipeElse->send(event);
                unsubscribe();
                locked->else_ = false;
            } else {
                pipeElse->send(event);
            }
        }
    });
}

EventFilter EventPipe::FilterOr(const EventFilter& filter1, const EventFilter& filter2) {
    return [filter1, filter2](const Event& event) {
        return filter1(event) || filter2(event);
    };
}

EventFilter EventPipe::FilterAnd(const EventFilter& filter1, const EventFilter& filter2) {
    return [filter1, filter2](const Event& event) {
        return filter1(event) && filter2(event);
    };
}

EventFilter EventPipe::FilterNot(const EventFilter& filter) {
    return [filter](const Event& event) {
        return !filter(event);
    };
}

EventFilter EventPipe::FilterTopic(size_t topic) {
    return [topic](const Event& event) {
        return event.topic == topic;
    };
}
