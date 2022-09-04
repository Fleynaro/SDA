#pragma once
#include "Binding.h"
#include "Call.h"
#include "Core/Context.h"

namespace sda::bind
{

    class ContextCallbacksBind : public Binding
    {
        using Callbacks = Context::Callbacks;

        class CallbacksJs : public Callbacks {
        public:
            ContextCallbacksBind* m_bindObject;

            void onObjectAdded(Object* obj) override {
                Call(m_bindObject, "onObjectAdded")(100);
            }
        };
    public:
        std::shared_ptr<Callbacks> m_instance;

        ContextCallbacksBind(std::shared_ptr<Callbacks> instance)
            : m_instance(instance)
        {
            init(m_instance.get());
        }

        ContextCallbacksBind() {
            auto isolate = v8::Isolate::GetCurrent();
            auto thisObject = v8pp::to_v8(isolate, this);

            auto callbacksJs = std::make_shared<CallbacksJs>();
            callbacksJs->m_bindObject = this;
            m_instance = callbacksJs;
            init(m_instance.get());
        }

        static void Init(v8pp::module& module) {
            v8pp::class_<ContextCallbacksBind> cl(module.isolate());
            cl.ctor<>();
            module.class_("ContextCallbacks", cl);
        }
    };

    class ContextBind : public Binding
    {
    public:
        Context* m_instance;

        ContextBind(Context* instance)
            : m_instance(instance)
        {
            init(m_instance);
        }

        ContextBind() {
            m_instance = new Context(nullptr);
            init(m_instance);
        }

        void setCallbacks(ContextCallbacksBind* callbacks) {
            m_instance->setCallbacks(callbacks->m_instance);
        }

        ContextCallbacksBind* getCallbacks() {
            return Get<ContextCallbacksBind>(m_instance->getCallbacks().get());
        }

        static void Init(v8pp::module& module) {
            v8pp::class_<ContextBind> cl(module.isolate());
            cl.ctor<>();
            cl.property("callbacks", &getCallbacks, &setCallbacks);
            module.class_("Context", cl);
        }
    };
};