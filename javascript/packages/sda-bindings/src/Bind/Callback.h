#pragma once
#include <v8pp/call_v8.hpp>

namespace sda::bind
{
    class Callback
    {
        std::unique_ptr<v8::Global<v8::Function>> m_callback;
    public:
        Callback() {
            m_callback = std::make_unique<v8::Global<v8::Function>>();
        }

        template<typename R = void, typename... Args>
        R call(Args&&... args) const {
            auto isolate = v8::Isolate::GetCurrent();
            auto func = m_callback->Get(isolate);
            if (func.IsEmpty()) {
                throw std::runtime_error("Callback is empty");
            }
            auto context = isolate->GetCurrentContext();
            auto result = v8pp::call_v8(isolate, func, context->Global(), args...);
            if constexpr (!std::is_same_v<R, void>) {
                return v8pp::from_v8<R>(isolate, result);
            }
        }

        bool isDefined() const {
            return !m_callback->IsEmpty();
        }

        template<typename T, typename Traits>
        static void Register(v8pp::class_<T, Traits>& cl, const std::string& name, Callback T::*pCallback) {
            cl.property(name, [pCallback](T& self) {
                // get
                auto& callback = self.*pCallback;
                return callback.m_callback->Get(v8::Isolate::GetCurrent());
            }, [pCallback](T& self, v8::Local<v8::Value> value) {
                // set
                auto isolate = v8::Isolate::GetCurrent();
                if (!value->IsFunction()) {
                    v8pp::throw_ex(isolate, "Value is not a function");
                }
                v8::Local<v8::Function> func = value.As<v8::Function>();
                auto& callback = self.*pCallback;
                callback.m_callback->Reset(v8::Isolate::GetCurrent(), func);
            });
        }
    };
};