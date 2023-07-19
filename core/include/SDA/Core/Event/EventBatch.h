#pragma once
#include <list>
#include "Event.h"

namespace sda
{
    template<typename T>
    struct EventBatch : Event {
        std::list<T> events;

        EventBatch(size_t topic)
            : Event(topic)
        {}
    };
};