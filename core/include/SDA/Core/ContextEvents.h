#pragma once
#include "SDA/Core/Event/Event.h"
#include "Context.h"

namespace sda
{
    static const size_t ContextEventTopic = TopicName("ContextEventTopic");

    struct ObjectActionEvent : Event {
        Object* object;

        ObjectActionEvent(Object* object)
            : Event(ContextEventTopic)
            , object(object)
        {}
    };

    // When an object is added to the context
    struct ObjectAddedEvent : ObjectActionEvent {
        using ObjectActionEvent::ObjectActionEvent;
    };

    // When an object is modified in the context
    struct ObjectModifiedEvent : ObjectActionEvent {
        using ObjectActionEvent::ObjectActionEvent;
    };

    // When an object is removed from the context
    struct ObjectRemovedEvent : ObjectActionEvent {
        using ObjectActionEvent::ObjectActionEvent;
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