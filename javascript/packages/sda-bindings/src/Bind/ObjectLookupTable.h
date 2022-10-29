#pragma once
#include <v8pp/class.hpp>

namespace sda::bind
{
    template<typename T>
    class ObjectLookupTableRaw
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

        static void Register(v8pp::class_<T, v8pp::raw_ptr_traits>& cl) {
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

    template<typename T>
    class ObjectLookupTableShared
    {
        using Hash = size_t;
        static inline std::unordered_map<Hash, std::weak_ptr<T>> Table;
        static inline bool IsRegistered = false;
    public:
        static void AddObject(std::shared_ptr<T> obj) {
            if (!IsRegistered)
                return;
            Table[GetHash(obj.get())] = obj;
        }

        static void RemoveObject(std::shared_ptr<T> obj) {
            if (!IsRegistered)
                return;
            Table.erase(GetHash(obj.get()));
        }

        static void Register(v8pp::class_<T, v8pp::shared_ptr_traits>& cl) {
            cl
                .property("hashId", [](T& self) { return GetHash(&self); })
                .static_method("Get", &GetObject);
            IsRegistered = true;
        }

        // TODO: clean up table (empty weak_ptr) by timer
        
    private:
        static Hash GetHash(T* obj) {
            return Hash(obj);
        }

        static std::shared_ptr<T> GetObject(Hash hash) {
            auto it = Table.find(hash);
            if (it != Table.end()) {
                return it->second.lock();
            }
            return nullptr;
        }
    };
};