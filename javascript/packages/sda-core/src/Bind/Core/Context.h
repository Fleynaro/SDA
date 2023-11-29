#pragma once
#include "SDA/Core/ContextInclude.h"
#include "SDA/Core/ContextEvents.h"
#include "Event.h"

namespace sda::bind
{
    class ContextEventBind : public EventBind
    {
        static void ObjectActionEventInit(v8pp::module& module) {
            auto cl = NewClass<ObjectActionEvent>(module);
            cl
                .auto_wrap_objects(true)
                .inherit<Event>()
                .property("object", [](ObjectActionEvent& self) { return self.object; });
            module.class_("ObjectActionEvent", cl);
        }

        static void ObjectAddedEventInit(v8pp::module& module) {
            auto cl = NewClass<ObjectAddedEvent>(module);
            cl
                .auto_wrap_objects(true)
                .inherit<ObjectActionEvent>();
            module.class_("ObjectAddedEvent", cl);
            RegisterEvent<ObjectAddedEvent>();
        }

        static void ObjectModifiedEventInit(v8pp::module& module) {
            auto cl = NewClass<ObjectModifiedEvent>(module);
            cl
                .auto_wrap_objects(true)
                .inherit<ObjectActionEvent>()
                .property("state", [](ObjectModifiedEvent& self) { return self.state; });
            module.class_("ObjectModifiedEvent", cl);
            RegisterEvent<ObjectModifiedEvent>();
        }

        static void ObjectRemovedEventInit(v8pp::module& module) {
            auto cl = NewClass<ObjectRemovedEvent>(module);
            cl
                .auto_wrap_objects(true)
                .inherit<ObjectActionEvent>();
            module.class_("ObjectRemovedEvent", cl);
            RegisterEvent<ObjectRemovedEvent>();
        }

        static void ContextRemovedEventInit(v8pp::module& module) {
            auto cl = NewClass<ContextRemovedEvent>(module);
            cl
                .auto_wrap_objects(true)
                .inherit<Event>();
            module.class_("ContextRemovedEvent", cl);
            RegisterEvent<ContextRemovedEvent>();
        }
    public:
        static void Init(v8pp::module& module) {
            ObjectActionEventInit(module);
            ObjectAddedEventInit(module);
            ObjectModifiedEventInit(module);
            ObjectRemovedEventInit(module);
            ContextRemovedEventInit(module);
        }
    };

    class ContextBind
    {
        static auto MakeSyncWithLookupTable(Context* context) {
            auto ctxPipe = context->getEventPipe();
            auto pipe = ctxPipe->connect(EventPipe::New());
            pipe->subscribe(std::function([context](const ObjectAddedEvent& event) {
                if (event.object->getContext() != context) return;
                ObjectLookupTableRaw::AddObject(event.object);
            }));
            pipe->subscribe(std::function([context](const ObjectRemovedEvent& event) {
                if (event.object->getContext() != context) return;
                ObjectLookupTableRaw::RemoveObject(event.object);
                RemoveObjectRef(event.object);
            }));
            pipe->subscribe(std::function([context, ctxPipe, pipe](const ContextRemovedEvent& event) {
                if (event.context != context) return;
                ObjectLookupTableRaw::RemoveObject(event.context);
                ctxPipe->disconnect(pipe);
            }));
        }

        static auto New(std::shared_ptr<EventPipe> pipe, Platform* platform) {
            auto context = new Context(pipe, platform);
            context->initDefault();
            MakeSyncWithLookupTable(context);
            ObjectLookupTableRaw::AddObject(context);
            return ExportObject(context);
        }

    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Context>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &Context::getEventPipe)
                .property("platform", &Context::getPlatform)
                .property("addressSpaces", [](Context& self) {
                    std::list<AddressSpace*> list;
                    for (auto addressSpace : *self.getAddressSpaces())
                        list.push_back(addressSpace);
                    return list;
                })
                .static_method("New", &New);
            ObjectLookupTableRaw::Register(cl);
            RegisterClassName(cl, "Context");
            module.class_("Context", cl);
        }
    };
};