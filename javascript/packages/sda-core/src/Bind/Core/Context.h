#pragma once
#include "SDA/Core/ContextInclude.h"

namespace sda::bind
{
    class ContextCallbacksBind
    {
        using Callbacks = Context::Callbacks;
        class CallbacksJsImpl : public Callbacks
        {
        public:
            std::shared_ptr<Callbacks> m_oldCallbacks;
            Callback m_onObjectAdded;
            Callback m_onObjectModified;
            Callback m_onObjectRemoved;
            
            void onObjectAdded(Object* obj) override {
                m_oldCallbacks->onObjectAdded(obj);
                if (m_onObjectAdded.isDefined()) {
                    m_onObjectAdded.call(obj);
                }
            }

            void onObjectModified(Object* obj) override {
                m_oldCallbacks->onObjectModified(obj);
                if (m_onObjectModified.isDefined()) {
                    m_onObjectModified.call(obj);
                }
            }

            void onObjectRemoved(Object* obj) override {
                m_oldCallbacks->onObjectRemoved(obj);
                if (m_onObjectRemoved.isDefined()) {
                    m_onObjectRemoved.call(obj);
                }
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
                    .auto_wrap_object_ptrs(true)
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
                Callback::Register(cl, "onObjectAdded", &CallbacksJsImpl::m_onObjectAdded);
                Callback::Register(cl, "onObjectModified", &CallbacksJsImpl::m_onObjectModified);
                Callback::Register(cl, "onObjectRemoved", &CallbacksJsImpl::m_onObjectRemoved);
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
                .auto_wrap_object_ptrs(true)
                .property("callbacks", &Context::getCallbacks, &Context::setCallbacks)
                .property("addressSpaces", [](Context& self) {
                    std::list<AddressSpace*> list;
                    for (auto addressSpace : *self.getAddressSpaces())
                        list.push_back(addressSpace);
                    return list;
                })
                .static_method("New", &New);
            ObjectLookupTableRaw::Register(cl);
            module.class_("Context", cl);
        }
    };
};