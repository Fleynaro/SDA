#pragma once
#include <v8pp/class.hpp>

namespace sda::bind
{
    template<typename T>
    class ObjectLookupTable
    {
        using Hash = size_t;
        static inline std::unordered_map<Hash, T*> Table;
        static inline bool IsRegistered = false;
    public:
        static void AddObject(T* obj) {
            if (!IsRegistered)
                return;
            Table[GetHash(obj)] = obj;
        }

        static void RemoveObject(T* obj) {
            if (!IsRegistered)
                return;
            Table.erase(GetHash(obj));
        }

        template<typename Traits = v8pp::raw_ptr_traits>
        static void Register(v8pp::class_<T, Traits>& cl) {
            cl
                .property("hashId", [](T& self) { return GetHash(&self); })
                .static_method("Get", &GetObject);
            IsRegistered = true;
        }
        
    private:
        static Hash GetHash(T* obj) {
            return Hash(obj);
        }

        static T* GetObject(Hash hash) {
            auto it = Table.find(hash);
            if (it != Table.end()) {
                return it->second;
            }
            return nullptr;
        }
    };
};