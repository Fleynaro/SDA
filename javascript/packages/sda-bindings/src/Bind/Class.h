#pragma once
#include <functional>

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
};

#define AsMemberFunction(func) sda::bind::AsMemberFunction<__COUNTER__>(std::function(func))