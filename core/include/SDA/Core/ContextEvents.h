#pragma once
#include "SDA/Core/Event/Event.h"
#include "Context.h"

namespace sda
{
    static const size_t ContextEventTopic = TopicName("ContextEventTopic");

    // When an object is added to the context
    struct ObjectAddedEvent : Event {
        Object* object;

        ObjectAddedEvent(Object* object)
            : Event(ContextEventTopic)
            , object(object)
        {}
    };

    // When an object is modified in the context
    struct ObjectModifiedEvent : Event {
        Object* object;

        ObjectModifiedEvent(Object* object)
            : Event(ContextEventTopic)
            , object(object)
        {}
    };

    // When an object is removed from the context
    struct ObjectRemovedEvent : Event {
        Object* object;

        ObjectRemovedEvent(Object* object)
            : Event(ContextEventTopic)
            , object(object)
        {}
    };

    // When context is destroyed
    struct ContextDestroyedEvent : Event {
        Context* context;

        ContextDestroyedEvent(Context* context)
            : Event(ContextEventTopic)
            , context(context)
        {}
    };
};