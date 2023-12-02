#include <v8pp/convert.hpp>
#include <filesystem>

namespace sda::bind
{
    template<typename T>
    auto to_v8(const std::list<std::unique_ptr<T>>& list) {
        std::list<v8::Local<v8::Object>> result;
        auto isolate = v8::Isolate::GetCurrent();
        std::transform(list.begin(), list.end(), std::back_inserter(result), [&](auto& p) {
            return v8pp::to_v8(isolate, p.get());
        });
        return result;
    }
};