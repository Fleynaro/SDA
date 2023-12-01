#pragma once
#include "SDA/Core/Event/EventPipe.h"

namespace sda::bind
{
    struct JsEvent : Event {
        v8::Global<v8::Object> object;
        JsEvent(size_t topic, v8::Local<v8::Object> object)
            : Event(topic), object(v8::Isolate::GetCurrent(), object)
        {}
    };

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
            if (auto e = dynamic_cast<const JsEvent*>(&event)) {
                return v8pp::to_local(isolate, e->object);
            }
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
            module.class_("SdaEvent", cl);
            UnknownEventInit(module);
        }
    };

    class EventPipeBind
    {
        static std::shared_ptr<v8::Global<v8::Function>> ToFunction(v8::Local<v8::Value> value) {
            auto isolate = v8::Isolate::GetCurrent();
            if (!value->IsFunction()) {
                throw std::runtime_error("Argument is not a function");
            }
            auto function = std::make_shared<v8::Global<v8::Function>>();
            function->Reset(isolate, value.As<v8::Function>());
            return function;
        }

        static JsEvent ToJsEvent(v8::Local<v8::Object> event) {
            auto isolate = v8::Isolate::GetCurrent();
            size_t topic;
            if (!v8pp::get_option(isolate, event, "topic", topic))
                throw std::runtime_error("Event has no topic");
            return JsEvent(topic, event);
        }

        static auto Process(EventPipe* pipe, v8::Local<v8::Value> value) {
            auto function = ToFunction(value);
            return pipe->process([function](const Event& event, const EventNext& next) {
                auto isolate = v8::Isolate::GetCurrent();
                auto func = function->Get(isolate);
                auto context = isolate->GetCurrentContext();
                auto eventObj = EventBind::ToObject(event);
                auto nextFunc = v8pp::wrap_function(isolate, "", [next](v8::Local<v8::Object> event) {
                    next(ToJsEvent(event));
                });
                v8pp::call_v8(isolate, func, context->Global(), eventObj, nextFunc);
            });
        }

        static auto Filter(EventPipe* pipe, v8::Local<v8::Value> value) {
            auto function = ToFunction(value);
            return pipe->filter([function](const Event& event) {
                auto isolate = v8::Isolate::GetCurrent();
                auto func = function->Get(isolate);
                auto context = isolate->GetCurrentContext();
                auto result = v8pp::call_v8(isolate, func, context->Global(), EventBind::ToObject(event));
                return v8pp::from_v8<bool>(isolate, result);
            });
        }

        static auto Send(EventPipe* pipe, v8::Local<v8::Object> event) {
            pipe->send(ToJsEvent(event));
        }

        static auto Subscribe(EventPipe* pipe, v8::Local<v8::Value> value) {
            auto isolate = v8::Isolate::GetCurrent();
            auto function = ToFunction(value);
            auto unsubscribe = pipe->subscribe([function](const Event& event) {
                auto isolate = v8::Isolate::GetCurrent();
                auto func = function->Get(isolate);
                auto context = isolate->GetCurrentContext();
                v8pp::call_v8(isolate, func, context->Global(), EventBind::ToObject(event));
            });
            return v8pp::wrap_function(isolate, "", unsubscribe);
        }

        static auto If(v8::Local<v8::Value> value, std::shared_ptr<EventPipe> pipeThen, std::shared_ptr<EventPipe> pipeElse) {
            auto function = ToFunction(value);
            return EventPipe::If([function](const Event& event) {
                auto isolate = v8::Isolate::GetCurrent();
                auto func = function->Get(isolate);
                auto context = isolate->GetCurrentContext();
                auto result = v8pp::call_v8(isolate, func, context->Global(), EventBind::ToObject(event));
                return v8pp::from_v8<bool>(isolate, result);
            }, pipeThen, pipeElse);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<EventPipe, v8pp::shared_ptr_traits>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &EventPipe::getName)
                .method("connect", &EventPipe::connect)
                .method("disconnect", &EventPipe::disconnect)
                .method("process", &Process)
                .method("filter", &Filter)
                .method("send", &Send)
                .method("subscribe", &Subscribe)
                .static_method("New", &EventPipe::New)
                .static_method("Combine", &EventPipe::Combine)
                .static_method("If", &If);
            module.class_("EventPipe", cl);
        }
    };

    static void EventBindInit(v8pp::module& module) {
        EventBind::Init(module);
        EventPipeBind::Init(module);
    }
};