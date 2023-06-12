#include "SDA/Core/Event/EventPipe.h"

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
    return process([filter](const Event& event, const EventNext& next) {
        if (filter(event))
            next(event);
    });
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
    return std::make_shared<EventPipe>("Combine", [pipeIn, pipeOut](const Event& event, const EventNext& next) {
        auto unsubscribe = pipeOut->subscribe(next);
        pipeIn->send(event);
        unsubscribe();
    });
}

std::shared_ptr<EventPipe> EventPipe::If(
    const EventFilter& condition,
    std::shared_ptr<EventPipe> pipeThen,
    std::shared_ptr<EventPipe> pipeElse)
{
    return std::make_shared<EventPipe>("If", [condition, pipeThen, pipeElse](const Event& event, const EventNext& next) {
        if (condition(event)) {
            auto unsubscribe = pipeThen->subscribe(next);
            pipeThen->send(event);
            unsubscribe();
        } else {
            auto unsubscribe = pipeElse->subscribe(next);
            pipeElse->send(event);
            unsubscribe();
        }
    });
}