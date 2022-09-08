#pragma once
#include "Core/Context.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Core/DataType/DataType.h"
#include "Core/Symbol/Symbol.h"
#include "Core/SymbolTable/SymbolTable.h"

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
            auto callbacks = std::make_shared<CallbacksJsImpl>();
            return ExportObject(callbacks);
        }

    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Callbacks, v8pp::shared_ptr_traits> cl_base(module.isolate());
            cl_base
                .method("onObjectAdded", &Callbacks::onObjectAdded)
                .method("onObjectModified", &Callbacks::onObjectModified)
                .method("onObjectRemoved", &Callbacks::onObjectRemoved);
            module.class_("ContextCallbacks", cl_base);
            
            v8pp::class_<CallbacksJsImpl, v8pp::shared_ptr_traits> cl(module.isolate());
            cl
                .inherit<Callbacks>()
                .var("oldCallbacks", &CallbacksJsImpl::m_oldCallbacks)
                .static_method("New", &New);
            module.class_("ContextCallbacksImpl", cl);
        }
    };

    class ContextBind
    {
        class BindCallbacks : public Context::Callbacks {
            void onObjectAdded(Object* obj) override;

            void onObjectRemoved(Object* obj) override;
        };

        static auto New(Platform* platform) {
            auto context = new Context(std::unique_ptr<Platform>(platform));
            auto callbacks = std::make_shared<BindCallbacks>();
            ExportObject(std::static_pointer_cast<Context::Callbacks>(callbacks));
            context->setCallbacks(callbacks);
            return ExportObject(context);
        }

    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Context> cl(module.isolate());
            cl
                .property("callbacks", &Context::getCallbacks, &Context::setCallbacks)
                .static_method("New", &New);
            module.class_("Context", cl);
        }
    };
};