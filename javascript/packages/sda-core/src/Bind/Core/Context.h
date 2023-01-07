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
            Callback m_onObjectAddedImpl;
            Callback m_onObjectModifiedImpl;
            Callback m_onObjectRemovedImpl;

            std::string getName() const override {
                return "CustomJs";
            }
            
            void onObjectAddedImpl(Object* obj) override {
                if (m_onObjectAddedImpl.isDefined()) {
                    m_onObjectAddedImpl.call(obj);
                }
            }

            void onObjectModifiedImpl(Object* obj) override {
                if (m_onObjectModifiedImpl.isDefined()) {
                    m_onObjectModifiedImpl.call(obj);
                }
            }

            void onObjectRemovedImpl(Object* obj) override {
                if (m_onObjectRemovedImpl.isDefined()) {
                    m_onObjectRemovedImpl.call(obj);
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
                    .property("name", &Callbacks::getName)
                    .method("setPrevCallbacks", &Callbacks::setPrevCallbacks)
                    .method("setEnabled", &Callbacks::setEnabled)
                    .method("onObjectAdded", &Callbacks::onObjectAdded)
                    .method("onObjectModified", &Callbacks::onObjectModified)
                    .method("onObjectRemoved", &Callbacks::onObjectRemoved)
                    .static_method("Find", &Callbacks::Find);
                module.class_("ContextCallbacks", cl);
            }
            
            {
                auto cl = NewClass<CallbacksJsImpl, v8pp::shared_ptr_traits>(module);
                cl
                    .inherit<Callbacks>()
                    .static_method("New", &New);
                Callback::Register(cl, "onObjectAddedImpl", &CallbacksJsImpl::m_onObjectAddedImpl);
                Callback::Register(cl, "onObjectModifiedImpl", &CallbacksJsImpl::m_onObjectModifiedImpl);
                Callback::Register(cl, "onObjectRemovedImpl", &CallbacksJsImpl::m_onObjectRemovedImpl);
                module.class_("ContextCallbacksImpl", cl);
            }
        }
    };

    class ContextBind
    {
        class RefCallbacks : public Context::Callbacks {
            std::string getName() const override {
                return "Ref";
            }

            void onObjectAddedImpl(Object* obj) override {
                ExportObjectRef(obj);
            }

            void onObjectRemovedImpl(Object* obj) override {
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
                .property("platform", &Context::getPlatform)
                .property("callbacks", &Context::getCallbacks, &Context::setCallbacks)
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