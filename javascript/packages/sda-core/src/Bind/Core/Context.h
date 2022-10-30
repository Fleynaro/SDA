#pragma once
#include "SDA/Core/Context.h"
#include "SDA/Core/Image/AddressSpace.h"
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/Symbol/Symbol.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"

namespace sda::bind
{
    class ContextCallbacksBind
    {
        using Callbacks = Context::Callbacks;
        class CallbacksJsImpl : public Callbacks
        {
            using Call = Call<CallbacksJsImpl, v8pp::shared_ptr_traits>;
        public:
            std::shared_ptr<Callbacks> m_oldCallbacks;

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
        
        static auto New() {
            return std::make_shared<CallbacksJsImpl>();
        }

    public:
        static void Init(v8pp::module& module) {
            {
                auto cl = NewClass<Callbacks, v8pp::shared_ptr_traits>(module);
                cl
                    .method("onObjectAdded", &Callbacks::onObjectAdded)
                    .method("onObjectModified", &Callbacks::onObjectModified)
                    .method("onObjectRemoved", &Callbacks::onObjectRemoved);
                module.class_("ContextCallbacks", cl);
            }
            
            {
                auto cl = NewClass<CallbacksJsImpl, v8pp::shared_ptr_traits>(module);
                cl
                    .inherit<Callbacks>()
                    .var("oldCallbacks", &CallbacksJsImpl::m_oldCallbacks)
                    .static_method("New", &New);
                module.class_("ContextCallbacksImpl", cl);
            }
        }
    };

    class ContextBind
    {
        class RefCallbacks : public Context::Callbacks {
            void onObjectAdded(Object* obj) override {
                ExportObjectRef(obj);
            }

            void onObjectRemoved(Object* obj) override {
                RemoveObjectRef(obj);
            }
        };

        static auto New(Platform* platform) {
            auto context = new Context(platform);
            context->setCallbacks(std::make_shared<RefCallbacks>());
            return ExportObject(context);
        }

    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Context>(module);
            cl
                .inherit<utils::IWrappable>()
                .property("callbacks", &Context::getCallbacks, &Context::setCallbacks)
                .static_method("New", &New);
            module.class_("Context", cl);
        }
    };
};