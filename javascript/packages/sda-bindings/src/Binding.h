#pragma once
#include <v8pp/class.hpp>
#include <v8pp/module.hpp>
#include <v8pp/call_v8.hpp>

namespace sda::bind
{
    template<typename T, typename Traits = v8pp::raw_ptr_traits>
    class Call
    {
        const T* m_object;
        std::string m_methodName;
        v8::Local<v8::Value> m_result;
        bool m_hasMethod = false;
    public:
        Call(const T* object, const std::string& methodName)
            : m_object(object)
            , m_methodName(methodName)
        {}

        template<typename... Args>
        const Call& operator()(Args&&... args) {
            auto isolate = v8::Isolate::GetCurrent();
            auto thisObject = v8pp::class_<T, Traits>::find_object(isolate, *m_object);
            auto context = isolate->GetCurrentContext();
            auto funcName = v8pp::to_v8(isolate, m_methodName);
            v8::Local<v8::Value> value;
            if (thisObject->Get(context, funcName).ToLocal(&value) && value->IsFunction()) {
                v8::Local<v8::Function> func = value.As<v8::Function>();
                m_result = v8pp::call_v8(isolate, func, thisObject, args...);
                m_hasMethod = true;
            }
            m_result = v8::Undefined(isolate);
            m_hasMethod = false;
            return *this;
        }

        template<typename R>
        R result() const {
            auto isolate = v8::Isolate::GetCurrent();
            return v8pp::from_v8<R>(isolate, m_result);
        }

        bool hasMethod() const {
            return m_hasMethod;
        }
    };
};