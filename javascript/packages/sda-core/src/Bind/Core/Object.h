#pragma once
#include "Core/Object/ContextObject.h"

namespace sda::bind
{
    class ObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Object> cl(module.isolate());
            cl
                .inherit<utils::ISerializable>()
                .property("id", [](const Object& self) { return std::string(self.serializeId()); })
                .method("setTemporary", &Object::setTemporary);
            module.class_("Object", cl);
        }
    };

    class ContextObjectBind : public ObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<ContextObject> cl(module.isolate());
            cl
                .inherit<Object>()
                .property("name", &ContextObject::getName, &ContextObject::setName)
                .property("comment", &ContextObject::getComment, &ContextObject::setComment);
            module.class_("ContextObject", cl);
        }
    };
};