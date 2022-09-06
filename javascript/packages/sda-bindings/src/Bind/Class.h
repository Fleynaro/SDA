#pragma once
#include <v8pp/class.hpp>

namespace sda::bind
{
    template<size_t idx, typename T, typename R, typename... Args>
    static auto AsMemberFunction(const std::function<R(T*, Args...)>& func) {
        static auto SavedFunc = func;
        class Derived : public T {
        public:
            auto method(Args... args) {
                return SavedFunc(this, args...);
            }
        };
        return static_cast<R(Derived::*)(Args...)>(&Derived::method);
    }

    template<typename Tratis, typename T, typename R, typename... Args, std::size_t... Is>
    static auto Method(const std::function<R(T*, Args...)>& func, std::index_sequence<Is...>) {
        return [func](const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = args.GetIsolate();
            if (args.Length() != sizeof...(Args)) {
                v8pp::throw_ex(isolate, "Wrong number of arguments");
                return;
            }
            T* thisObject = nullptr;
            if constexpr (std::is_same_v<Tratis, v8pp::raw_ptr_traits>) {
                thisObject = v8pp::class_<T, Tratis>::unwrap_object(isolate, args.This());
            } else if constexpr (std::is_same_v<Tratis, v8pp::shared_ptr_traits>) {
                thisObject = v8pp::class_<T, Tratis>::unwrap_object(isolate, args.This()).get();
            } else if constexpr (!std::is_same_v<Tratis, void>) {
                static_assert(false, "Unsupported traits");
            }
            auto argTuple = std::make_tuple(thisObject, v8pp::from_v8<Args>(isolate, args[Is])...);
            if constexpr (std::is_same_v<R, void>) {
                std::apply(func, argTuple);
            } else {
                auto result = std::apply(func, argTuple);
                args.GetReturnValue().Set(v8pp::to_v8(isolate, result));
            }
        };
    }

    template<typename Tratis, typename T, typename R,  typename... Args>
    static auto Method(const std::function<R(T*, Args...)>& func) {
        return Method<Tratis>(func, std::make_index_sequence<sizeof...(Args)>());
    }

    template<typename Tratis, typename T, typename R,  typename... Args>
    static auto Method(R(T::*func)(Args...)) {
        return Method<Tratis>(std::function([func](T* thisObject, Args... args) {
            return (thisObject->*func)(args...);
        }));
    }

    template<typename T, typename... Args>
    static auto StaticMethod(const std::function<T(Args...)>& func) {
        return Method<void>(std::function([func](void* thisObject, Args... args) {
            return func(args...);
        }));
    }
};

#define AsMemberFunction(func) sda::bind::AsMemberFunction<__COUNTER__>(std::function(func))