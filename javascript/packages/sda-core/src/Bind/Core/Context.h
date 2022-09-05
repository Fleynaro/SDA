#pragma once
#include "Core/Context.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Core/DataType/DataType.h"
#include "Core/Symbol/Symbol.h"
#include "Core/SymbolTable/SymbolTable.h"
#include <iostream>

namespace sda::bind
{

    class ContextCallbacksBind
    {
        using Callbacks = Context::Callbacks;
    public:
        class CallbacksJsImpl : public Callbacks
        {
        public:
            std::shared_ptr<Callbacks> m_callbacks;
            std::shared_ptr<Callbacks> m_oldCallbacks;

            CallbacksJsImpl() {
                m_callbacks = std::shared_ptr<Callbacks>(this);
            }

            ~CallbacksJsImpl() {
                m_callbacks.reset();
            }

            void onObjectAdded(Object* obj) override {
                m_oldCallbacks->onObjectAdded(obj);
                Call(this, "onObjectAdded")(obj);
            }

            void onObjectModified(Object* obj) override {
                m_oldCallbacks->onObjectModified(obj);
                Call(this, "onObjectModified")(obj);
            }

            void onObjectRemoved(Object* obj) override {
                m_oldCallbacks->onObjectRemoved(obj);
                Call(this, "onObjectRemoved")(obj);
            }
        };
        
    private:
        static auto Create() {
            auto callbacks = new CallbacksJsImpl();
            return ExportObject(callbacks);
        }

    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Callbacks> cl_base(module.isolate());
            cl_base.function("onObjectAdded", &Callbacks::onObjectAdded);
            cl_base.function("onObjectModified", &Callbacks::onObjectModified);
            cl_base.function("onObjectRemoved", &Callbacks::onObjectRemoved);
            module.class_("ContextCallbacks", cl_base);
            
            v8pp::class_<CallbacksJsImpl> cl(module.isolate());
            cl.inherit<Callbacks>();
            cl.function("create", &Create);
            module.class_("ContextCallbacksImpl", cl);
        }
    };

    class ContextBind
    {
        class BindCallbacks : public Context::Callbacks {
            void onObjectAdded(Object* obj) override;

            void onObjectRemoved(Object* obj) override;
        };

        static auto Create() {
            auto context = new Context(nullptr);
            context->setCallbacks(std::make_shared<BindCallbacks>());
            return ExportObject(context);
        }

        static void SetCallbacks(Context* ctx, ContextCallbacksBind::CallbacksJsImpl* callbacks) {
            callbacks->m_oldCallbacks = ctx->getCallbacks();
            ctx->setCallbacks(callbacks->m_callbacks);
        }

    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Context> cl(module.isolate());
            cl.function("create", &Create);
            cl.function("setCallbacks", AsMemberFunction(&SetCallbacks));
            module.class_("Context", cl);
        }
    };
};