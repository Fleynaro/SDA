#pragma once
#include "Binding.h"
#include "Core/DataType/VoidDataType.h"

namespace sda::bind
{
    class ObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Object> cl(module.isolate());
            cl.property("id", &Object::getId);
            module.class_("Object", cl);
        }
    };

    class ContextObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<ContextObject> cl(module.isolate());
            cl.inherit<Object>();
            cl.property("name", &ContextObject::getName, &ContextObject::setName);
            cl.property("comment", &ContextObject::getComment, &ContextObject::setComment);
            module.class_("ContextObject", cl);
        }
    };

    class DataTypeBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<DataType> cl(module.isolate());
            cl.inherit<ContextObject>();
            cl.property("isVoid", &DataType::isVoid);
            cl.property("size", &DataType::getSize);
            module.class_("DataType", cl);
        }
    };

    class VoidDataTypeBind : public DataTypeBind
    {
        class DeferredCallbacks : public Context::Callbacks
        {
            std::list<Object*> m_objects;

            void onObjectAdded(Object* obj) override {
                m_objects.push_back(obj);
            }
        public:
            void complete(Context::Callbacks* callbacks) {
                for (auto obj : m_objects)
                    callbacks->onObjectAdded(obj);
            }
        };

        static auto Create(Context* ctx) {
            auto isolate = v8::Isolate::GetCurrent();
            auto oldCallbacks = ctx->getCallbacks();
            auto deferredCallbacks = std::make_shared<DeferredCallbacks>();
            ctx->setCallbacks(deferredCallbacks);
            auto dataType = new VoidDataType(ctx);
            auto object = v8pp::class_<VoidDataType>::import_external(isolate, dataType);
            ctx->setCallbacks(oldCallbacks);
            deferredCallbacks->complete(oldCallbacks.get());
            return object;
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<VoidDataType> cl(module.isolate());
            cl.inherit<DataType>();
            cl.function("create", &Create);
            module.class_("VoidDataType", cl);
        }
    };
};