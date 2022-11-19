#pragma once
#include "SDA/Core/Object/ContextObject.h"

namespace sda::bind
{
    class ObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Object>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .inherit<utils::ISerializable>()
                .property("id", [](const Object& self) { return std::string(self.serializeId()); })
                .method("setTemporary", &Object::setTemporary);
            ObjectLookupTableRaw::Register(cl);
            RegisterClassName(cl, "Object");
            module.class_("Object", cl);
        }
    };

    class ContextObjectBind : public ObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ContextObject>(module);
            cl
                .inherit<Object>()
                .property("name", &ContextObject::getName, &ContextObject::setName)
                .property("comment", &ContextObject::getComment, &ContextObject::setComment);
            module.class_("ContextObject", cl);
        }
    };
};