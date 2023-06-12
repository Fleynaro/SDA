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

    class EventPipe {
        EventProcessor m_processor;
        std::list<std::shared_ptr<EventPipe>> m_nextPipes;
    public:
        EventPipe(const EventProcessor& processor);
        
        void send(const Event& event);

        std::shared_ptr<EventPipe> process(const EventProcessor& processor);

        std::shared_ptr<EventPipe> filter(const EventFilter& filter);
        
        std::shared_ptr<EventPipe> handle(const EventHandler& handler);

        template<typename T>
        std::shared_ptr<EventPipe> handle(const std::function<void(const T&)>& handler) {
            return handle([handler](const Event& event) {
                if (auto e = dynamic_cast<const T*>(&event))
                    handler(*e);
            });
        }

        template<typename T, typename R>
        std::shared_ptr<EventPipe> handleMethod(R* instance, void (R::* method)(const T&)) {
            return handle<T>([method, instance](const T& event) {
                (instance->*method)(event);
            });
        }

        std::shared_ptr<EventPipe> connect(std::shared_ptr<EventPipe> pipe);

        void disconnect(std::shared_ptr<EventPipe> pipe);

        static std::shared_ptr<EventPipe> New();

        static std::shared_ptr<EventPipe> New(std::shared_ptr<EventPipe> pipeIn, std::shared_ptr<EventPipe> pipeOut);
    };
};