#include "SDA/Core/Event/EventPipe.h"

using namespace sda;

EventPipe::EventPipe(const EventProcessor& processor)
    : m_processor(processor)
{}

void EventPipe::send(const Event& event) {
    m_processor(event, [this](const Event& event) {
        for (auto& pipe : m_nextPipes)
            pipe.send(event);
    });
}

EventPipe& EventPipe::process(const EventProcessor& processor) {
    return connect(EventPipe(processor));
}

EventPipe& EventPipe::filter(const EventFilter& filter) {
    return process([filter](const Event& event, const EventNext& next) {
        if (filter(event))
            next(event);
    });
}

EventPipe& EventPipe::handle(const EventHandler& handler) {
    return process([handler](const Event& event, const EventNext& next) {
        handler(event);
        next(event);
    });
}

EventPipe& EventPipe::connect(const EventPipe& pipe) {
    m_nextPipes.emplace_back(pipe);
    return m_nextPipes.back();
}

void EventPipe::disconnect(const EventPipe& pipe) {
    m_nextPipes.remove_if([&pipe](const EventPipe& p) { return &p == &pipe; });
}