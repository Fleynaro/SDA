#pragma once
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
        std::list<EventPipe> m_nextPipes;
    public:
        EventPipe(const EventProcessor& processor = [](const Event& e, const EventNext& f) { f(e); });

        void send(const Event& event);

        EventPipe& process(const EventProcessor& processor);

        EventPipe& filter(const EventFilter& filter);
        
        EventPipe& handle(const EventHandler& handler);

        template<typename T>
        EventPipe& handle(const std::function<void(const T&)>& handler) {
            return handle([handler](const Event& event) {
                if (auto e = dynamic_cast<const T*>(&event))
                    handler(*e);
            });
        }

        template<typename T, typename R>
        EventPipe& handleMethod(R* instance, void (R::* method)(const T&)) {
            return handle<T>([method, instance](const T& event) {
                (instance->*method)(event);
            });
        }

        EventPipe& connect(const EventPipe& pipe);

        void disconnect(const EventPipe& pipe);
    };
};