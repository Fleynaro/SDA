#pragma once
#include "SDA/Core/Event/Event.h"
#include "Context.h"

namespace sda
{
    static const size_t ContextEventTopic = TopicName("ContextEventTopic");

    struct ObjectActionEvent : Event {
        ContextObject* object;

        ObjectActionEvent(ContextObject* object)
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
        Object::ModState state;
        
        ObjectModifiedEvent(ContextObject* object, Object::ModState state)
            : ObjectActionEvent(object)
            , state(state)
        {}
    };

    // When an object is removed from the context
    struct ObjectRemovedEvent : ObjectActionEvent {
        using ObjectActionEvent::ObjectActionEvent;
    };

    // When context is destroyed
    struct ContextRemovedEvent : Event {
        Context* context;

        ContextRemovedEvent(Context* context)
            : Event(ContextEventTopic)
            , context(context)
        {}
    };
};