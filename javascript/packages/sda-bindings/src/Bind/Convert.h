#include <magic_enum.hpp>
#include <v8pp/convert.hpp>
#include <filesystem>

namespace v8pp
{
    template<typename T>
    struct convert<T, typename std::enable_if<std::is_enum<T>::value>::type>
    {
        using underlying_type = std::string;

        using from_type = T;
        using to_type = typename convert<underlying_type>::to_type;

        static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value) {
            return convert<underlying_type>::is_valid(isolate, value);
        }

        static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value) {
            return magic_enum::enum_cast<T>(convert<underlying_type>::from_v8(isolate, value)).value();
        }

        static to_type to_v8(v8::Isolate* isolate, T value) {
            return convert<underlying_type>::to_v8(isolate, magic_enum::enum_name(value).data());
        }
    };

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
    std::list<T*> to_v8(const std::list<std::unique_ptr<T>>& list) {
        std::list<T*> result;
        std::transform(list.begin(), list.end(), std::back_inserter(result), [](auto& p) { return p.get(); });
        return result;
    }
};