#pragma once
#include "Core/Object/ContextObject.h"

namespace sda::bind
{
    class ObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Object> cl(module.isolate());
            cl.property("id", [](const Object& self) { return std::string(self.serializeId()); });
            cl.function("setTemporary", &Object::setTemporary);
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

    v8::Local<v8::Object> CreateContextObject(Context* ctx, std::function<v8::Local<v8::Object>()> creator) {
        class DeferredCallbacks : public Context::Callbacks
        {
            std::list<Object*> m_objectsAdded;

            void onObjectAdded(Object* obj) override {
                m_objectsAdded.push_back(obj);
            }
        public:
            void complete(Context::Callbacks* callbacks) {
                for (auto obj : m_objectsAdded)
                    callbacks->onObjectAdded(obj);
            }
        };

        auto oldCallbacks = ctx->getCallbacks();
        auto deferredCallbacks = std::make_shared<DeferredCallbacks>();
        ctx->setCallbacks(deferredCallbacks);

        auto object = creator();

        ctx->setCallbacks(oldCallbacks);
        deferredCallbacks->complete(oldCallbacks.get());
        return object;
    }
};