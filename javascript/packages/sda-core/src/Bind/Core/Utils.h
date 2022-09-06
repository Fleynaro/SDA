#pragma once
#include "Core/Utils/Serialization.h"

namespace sda::bind
{
    class SerializationBind
    {
        static auto Serialize(utils::ISerializable* obj) {
            boost::json::object data;
            obj->serialize(data);
            auto isolate = v8::Isolate::GetCurrent();
            return v8pp::json_parse(isolate, boost::json::serialize(data));
        }

        static void Deserialize(utils::ISerializable* obj, v8::Local<v8::Value> value) {
            auto isolate = v8::Isolate::GetCurrent();
            if (!value->IsObject()) {
                v8pp::throw_ex(isolate, "Invalid argument: expected object");
                return;
            }
            auto json = v8pp::json_str(isolate, value);
            boost::json::object data = boost::json::parse(json).as_object();
            obj->deserialize(data);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<utils::ISerializable> cl(module.isolate());
            cl
                .method("serialize", &Serialize)
                .method("deserialize", &Deserialize);
            module.class_("Serialization", cl);
        }
    };
};