#pragma once
#include "SDA/Core/Event/EventPipe.h"

namespace sda::bind
{
    class EventBind
    {
        using CasterType = bool(const Event&, v8::Local<v8::Object>& object);
        struct Data {
            std::list<std::function<CasterType>> casters;
        };

        static Data* GetData() {
            void* data = SharedData::Get<EventBind>();
            if (data == nullptr) {
                data = new Data();
                SharedData::Set<EventBind>(data);
            }
            return static_cast<Data*>(data);
        }

        struct UnknownEvent : Event {
            UnknownEvent(size_t topic) : Event(topic) {}
        };

        static void UnknownEventInit(v8pp::module& module) {
            auto cl = NewClass<UnknownEvent>(module);
            cl
                .auto_wrap_objects(true)
                .inherit<Event>();
            module.class_("UnknownEvent", cl);
        }
    protected:
        template<typename T>
        static void RegisterEvent() {
            GetData()->casters.push_back([](const Event& event, v8::Local<v8::Object>& object) {
                auto isolate = v8::Isolate::GetCurrent();
                if (auto ptr = dynamic_cast<const T*>(&event)) {
                    object = v8pp::to_v8<T>(isolate, *ptr);
                    return true;
                }
                return false;
            });
        }
    public:
        static v8::Local<v8::Object> ToObject(const Event& event) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::EscapableHandleScope scope(isolate);
            for (auto& caster : GetData()->casters) {
                v8::Local<v8::Object> object;
                if (caster(event, object)) {
                    return scope.Escape(object);
                }
            }
            return scope.Escape(v8pp::to_v8<UnknownEvent>(isolate, UnknownEvent(event.topic)));
        }

        static void Init(v8pp::module& module) {
            auto cl = NewClass<Event>(module);
            cl
                .property("topic", [](const Event& self) { return self.topic; });
            module.class_("Event", cl);
            UnknownEventInit(module);
        }
    };

    class EventPipeBind
    {
        static auto Subscribe(EventPipe* pipe, v8::Local<v8::Value> value) {
            auto isolate = v8::Isolate::GetCurrent();
            if (!value->IsFunction()) {
                throw std::runtime_error("Argument is not a function");
            }
            auto function = std::make_shared<v8::Global<v8::Function>>();
            function->Reset(isolate, value.As<v8::Function>());
            auto unsubscribe = pipe->subscribe([function](const Event& event) {
                auto isolate = v8::Isolate::GetCurrent();
                auto func = function->Get(isolate);
                if (func.IsEmpty()) {
                    throw std::runtime_error("Function is empty");
                }
                auto context = isolate->GetCurrentContext();
                v8pp::call_v8(isolate, func, context->Global(), EventBind::ToObject(event));
            });
            return v8pp::wrap_function(isolate, "", unsubscribe);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<EventPipe, v8pp::shared_ptr_traits>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &EventPipe::getName)
                .method("connect", &EventPipe::connect)
                .method("disconnect", &EventPipe::disconnect)
                .method("subscribe", &Subscribe)
                .static_method("New", &EventPipe::New)
                .static_method("Combine", &EventPipe::Combine);
            module.class_("EventPipe", cl);
        }
    };
};