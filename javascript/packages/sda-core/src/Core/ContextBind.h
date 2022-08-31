#pragma once
#include "Core/Context.h"
#include <v8pp/class.hpp>
#include <v8pp/module.hpp>

namespace sda::bind
{
    class ContextBind
    {
    public:
        Context* m_instance;

        ContextBind(Context* instance)
            : m_instance(instance)
        {}

        ContextBind(v8::FunctionCallbackInfo<v8::Value> const& args) {
            m_instance = new Context(nullptr); 
        }

        static void Init(v8pp::module& module) {
            v8pp::class_<ContextBind> cl(module.isolate());
            cl.ctor<v8::FunctionCallbackInfo<v8::Value> const&>();
            module.class_("Context", cl);
        }
    };
};