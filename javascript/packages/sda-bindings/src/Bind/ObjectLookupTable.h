#pragma once
#include <v8pp/class.hpp>
#include "Shared/SharedData.h"

namespace sda::bind
{
    template<typename T>
    struct ObjectLookupTableInfo {};

    template<>
    struct ObjectLookupTableInfo<v8pp::raw_ptr_traits> {
        using stored_type = void*;

        template<typename T>
        static T* to_result_type(stored_type obj) {
            return static_cast<T*>(obj);
        }
    };

    template<>
    struct ObjectLookupTableInfo<v8pp::shared_ptr_traits> {
        using stored_type = std::weak_ptr<void>;

        template<typename T>
        static std::shared_ptr<T> to_result_type(stored_type obj) {
            if (auto ptr = obj.lock()) {
                return std::static_pointer_cast<T>(ptr);
            }
            return nullptr;
        }
    };

    template<typename Traits>
    class ObjectLookupTable
    {
        using stored_type = typename ObjectLookupTableInfo<Traits>::stored_type;
        using object_id = typename Traits::object_id;
        using pointer_type = typename Traits::pointer_type;
        template<typename T>
        using object_pointer_type = typename Traits::template object_pointer_type<T>;
        using Hash = size_t;
        using Table = std::unordered_map<Hash, stored_type>;
    public:
        static void AddObject(pointer_type obj) {
            auto id = Traits::pointer_id(obj);
            auto table = getTable();
            (*table)[GetHash(id)] = obj;
        }
        
        static void RemoveObject(pointer_type obj) {
            auto id = Traits::pointer_id(obj);
            auto table = getTable();
            table->erase(GetHash(id));
        }

        template<typename T>
        static void Register(v8pp::class_<T, Traits>& cl) {
            cl
                .property("hashId", [](T& self) { return GetHash(&self); })
                .static_method("Get", &GetObject<T>);
        }
        
    protected:
        static Table* getTable() {
            void* table = SharedData::Get<ObjectLookupTable>();
            if (table == nullptr) {
                table = new Table();
                SharedData::Set<ObjectLookupTable>(table);
            }
            return static_cast<Table*>(table);
        }

        static Hash GetHash(object_id id) {
            return Hash(id);
        }

        template<typename T>
        static object_pointer_type<T> GetObject(Hash hash) {
            auto table = getTable();
            auto it = table->find(hash);
            if (it != table->end()) {
                return ObjectLookupTableInfo<Traits>::to_result_type<T>(it->second);
            }
            return nullptr;
        }
    };

    class ObjectLookupTableRaw : public ObjectLookupTable<v8pp::raw_ptr_traits> {};

    class ObjectLookupTableShared : public ObjectLookupTable<v8pp::shared_ptr_traits>
    {
    public:
        static void Init(v8pp::module& module) {
            module.function("CleanUpSharedObjectLookupTable", &CleanUp);
        }
        
    private:
        static void CleanUp() {
            auto table = getTable();
            for (auto& [hash, obj] : *table) {
                if (obj.expired()) {
                    table->erase(hash);
                }
            }
        }
    };
};