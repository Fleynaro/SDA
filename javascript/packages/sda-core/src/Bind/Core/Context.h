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
            auto callbacks = std::make_shared<CallbacksJsImpl>();
            auto obj = ExportObject(callbacks);

            //auto isolate = v8::Isolate::GetCurrent();
            //auto thisObject = v8pp::class_<CallbacksJsImpl, v8pp::shared_ptr_traits>::unwrap_object(isolate, obj);
        }

    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Callbacks, v8pp::shared_ptr_traits> cl_base(module.isolate());
            cl_base.function("onObjectAdded", Method<v8pp::shared_ptr_traits>(&Callbacks::onObjectAdded));
            //cl_base.function("onObjectModified", &Callbacks::onObjectModified);
            //cl_base.function("onObjectRemoved", &Callbacks::onObjectRemoved);

            // cl_base.property("test",
            // [](Callbacks& callbacks) {
            //     return 5;
            // },
            // [](Callbacks& callbacks, Object* obj) {
            //     std::cout << "test = " << std::endl;
            // });
            // cl_base.function("onObjectAdded", Method<v8pp::shared_ptr_traits>(std::function([](Callbacks* callbacks, ContextObject* obj) {
            //     std::cout << "onObjectAdded name = " << obj->getName() << std::endl;
            // })));
            // cl_base.function("Statis", StaticMethod(std::function([](ContextObject* obj) {
            //     std::cout << "onObjectAdded name = " << obj->getName() << std::endl;
            // })));
            module.class_("ContextCallbacks", cl_base);
            
            v8pp::class_<CallbacksJsImpl, v8pp::shared_ptr_traits> cl(module.isolate());
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
            auto callbacks = std::make_shared<BindCallbacks>();
            ExportObject(std::static_pointer_cast<Context::Callbacks>(callbacks));
            context->setCallbacks(callbacks);
            return ExportObject(context);
        }

        static void SetCallbacks(Context& ctx, ContextCallbacksBind::CallbacksJsImpl* callbacks) {
            callbacks->m_oldCallbacks = ctx.getCallbacks();
            ctx.setCallbacks(callbacks->m_callbacks);
        }

    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Context> cl(module.isolate());
            cl.function("create", &Create);
            cl.property("callbacks", &Context::getCallbacks, &Context::setCallbacks);
            module.class_("Context", cl);
        }
    };
};