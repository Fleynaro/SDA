#include <v8pp/convert.hpp>
#include <filesystem>

namespace v8pp
{
    //for filesysyem::path
    template<>
    struct convert<std::filesystem::path>
    {
        using underlying_type = std::string;

        using from_type = std::filesystem::path;
        using to_type = typename convert<underlying_type>::to_type;

        static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value) {
            return convert<underlying_type>::is_valid(isolate, value);
        }

        static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value) {
            underlying_type path = convert<underlying_type>::from_v8(isolate, value);
            return path;
        }

        static to_type to_v8(v8::Isolate* isolate, const from_type& value) {
            return convert<underlying_type>::to_v8(isolate, value.string());
        }
    };
};

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