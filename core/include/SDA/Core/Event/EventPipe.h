#pragma once
#include <memory>
#include <functional>
#include "Event.h"

namespace sda
{
    using EventNext = std::function<void(const Event&)>;
    using EventProcessor = std::function<void(const Event&, const EventNext&)>;
    using EventFilter = std::function<bool(const Event&)>;
    using EventHandler = std::function<void(const Event&)>;
    using EventUnsubscribe = std::function<void()>;

    class EventPipe {
        std::string m_name;
        EventProcessor m_processor;
        std::list<EventHandler> m_handlers;
        std::list<std::shared_ptr<EventPipe>> m_nextPipes;
    public:
        EventPipe(const std::string& name, const EventProcessor& processor);
        
        void send(const Event& event);

        std::shared_ptr<EventPipe> process(const EventProcessor& processor);

        std::shared_ptr<EventPipe> filter(const EventFilter& filter);

        template<typename T>
        std::shared_ptr<EventPipe> filter(const std::function<bool(const T&)>& filter) {
            return this->filter([filter](const Event& event) {
                if (auto e = dynamic_cast<const T*>(&event))
                    return filter(*e);
                return false;
            });
        }
        
        EventUnsubscribe subscribe(const EventHandler& handler);

        template<typename T>
        EventUnsubscribe subscribe(const std::function<void(const T&)>& handler) {
            return subscribe([handler](const Event& event) {
                if (auto e = dynamic_cast<const T*>(&event))
                    handler(*e);
            });
        }

        template<typename T, typename R>
        EventUnsubscribe subscribeMethod(R* instance, void (R::* method)(const T&)) {
            return subscribe<T>([method, instance](const T& event) {
                (instance->*method)(event);
            });
        }

        std::shared_ptr<EventPipe> connect(std::shared_ptr<EventPipe> pipe);

        void disconnect(std::shared_ptr<EventPipe> pipe);

        static std::shared_ptr<EventPipe> New(const std::string& name = "");

        static std::shared_ptr<EventPipe> Combine(std::shared_ptr<EventPipe> pipeIn, std::shared_ptr<EventPipe> pipeOut);

        static std::shared_ptr<EventPipe> If(
            const EventFilter& condition,
            std::shared_ptr<EventPipe> pipeThen,
            std::shared_ptr<EventPipe> pipeElse);
    };
};