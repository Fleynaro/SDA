#pragma once
#include "Core/DataType/VoidDataType.h"
#include "Core/DataType/DataTypeBind.h"
#include "Core/ContextBind.h"

namespace sda::bind
{
    class VoidDataTypeBind : public DataTypeBind
    {
    public:
        VoidDataType* m_instance;

        VoidDataTypeBind(VoidDataType* instance) {
            DataTypeBind::m_instance =
            m_instance =
            instance;
        }

        VoidDataTypeBind(ContextBind* ctx) {
            DataTypeBind::m_instance =
            m_instance =
            new VoidDataType(ctx->m_instance);
        }

        // v8::Local<v8::Object> test(v8::Local<v8::Function> cb) {
        //     auto isolate = v8::Isolate::GetCurrent();
        //     std::cout << "calling";
        //     auto result = v8pp::call_v8(isolate, cb, isolate->GetCurrentContext()->Global(), 5);
        //     std::cout << "result: " << v8pp::from_v8<bool>(isolate, result) << std::endl;

        //     v8::Local<v8::Object> obj = v8::Object::New(isolate);
        //     v8pp::set_option(isolate, obj, "a", 10);
        //     v8pp::set_option(isolate, obj, "result", result);
        //     return obj;
        // }

        static void Init(v8pp::module& module) {
            v8pp::class_<VoidDataTypeBind> cl(module.isolate());
            cl.ctor<ContextBind*>();
            cl.inherit<DataTypeBind>();
            module.class_("VoidDataType", cl);
        }
    };
};