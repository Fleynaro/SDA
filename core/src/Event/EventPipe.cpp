#include "SDA/Core/Event/EventPipe.h"

using namespace sda;

EventPipe::EventPipe(const EventProcessor& processor)
    : m_processor(processor)
{}

void EventPipe::send(const Event& event) {
    m_processor(event, [this](const Event& event) {
        for (auto& pipe : m_nextPipes)
            pipe->send(event);
    });
}

std::shared_ptr<EventPipe> EventPipe::process(const EventProcessor& processor) {
    return connect(std::make_shared<EventPipe>(processor));
}

std::shared_ptr<EventPipe> EventPipe::filter(const EventFilter& filter) {
    return process([filter](const Event& event, const EventNext& next) {
        if (filter(event))
            next(event);
    });
}

std::shared_ptr<EventPipe> EventPipe::handle(const EventHandler& handler) {
    return process([handler](const Event& event, const EventNext& next) {
        handler(event);
        next(event);
    });
}

std::shared_ptr<EventPipe> EventPipe::connect(std::shared_ptr<EventPipe> pipe) {
    m_nextPipes.emplace_back(pipe);
    return m_nextPipes.back();
}

void EventPipe::disconnect(std::shared_ptr<EventPipe> pipe) {
    m_nextPipes.remove(pipe);
}

std::shared_ptr<EventPipe> EventPipe::New() {
    return std::make_shared<EventPipe>([](const Event& event, const EventNext& next) {
        next(event);
    });
}

std::shared_ptr<EventPipe> EventPipe::New(std::shared_ptr<EventPipe> pipeIn, std::shared_ptr<EventPipe> pipeOut) {
    return std::make_shared<EventPipe>([pipeIn, pipeOut](const Event& event, const EventNext& next) {
        pipeIn->send(event);
        pipeOut->handle(next);
    });
}