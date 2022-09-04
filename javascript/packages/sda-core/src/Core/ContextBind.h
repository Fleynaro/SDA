#pragma once
#include "Binding.h"
#include "Core/Context.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Core/DataType/DataType.h"
#include "Core/Symbol/Symbol.h"
#include "Core/SymbolTable/SymbolTable.h"

namespace sda::bind
{

    class ContextCallbacksBind
    {
        using Callbacks = Context::Callbacks;

        class CallbacksJs : public Callbacks
        {
            using Call = Call<CallbacksJs, v8pp::shared_ptr_traits>;
        public:
            void onObjectAdded(Object* obj) override {
                Call(this, "onObjectAdded")(obj);
            }
        };

        static auto Create() {
            auto isolate = v8::Isolate::GetCurrent();
            auto callbacks = std::make_shared<CallbacksJs>();
            return v8pp::class_<CallbacksJs, v8pp::shared_ptr_traits>::import_external(isolate, callbacks);
        }

    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Callbacks, v8pp::shared_ptr_traits> cl_base(module.isolate());
            module.class_("ContextCallbacks", cl_base);
            v8pp::class_<CallbacksJs, v8pp::shared_ptr_traits> cl(module.isolate());
            cl.inherit<Callbacks>();
            cl.function("create", &Create);
            module.class_("ContextCallbacksImpl", cl);
        }
    };

    class ContextBind
    {
        static auto Create() {
            auto isolate = v8::Isolate::GetCurrent();
            auto context = new Context(nullptr);
            return v8pp::class_<Context>::import_external(isolate, context);
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