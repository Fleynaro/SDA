#pragma once
#include <map>
#include <string>

namespace sda::bind
{
    class SharedData
    {
        static std::map<size_t, void*> Data;
    public:
        static void* Get(size_t key);

        static void Set(size_t key, void* data);

        template<typename T>
        static size_t Key() {
            return std::hash<std::string>()(typeid(T).name());
        }

        template<typename T>
        static void* Get() {
            return Get(Key<T>());
        }

        template<typename T>
        static void Set(void* data) {
            Set(Key<T>(), data);
        }
    };
};