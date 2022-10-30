#pragma once
#include <v8pp/class.hpp>

namespace sda::bind
{
    /// As ExportObjectRef but delete memory for C++ object
    template<typename T>
    static auto ExportObject(T* obj) {
        ObjectLookupTableRaw::AddObject(obj);
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T, v8pp::raw_ptr_traits>::import_external(isolate, obj);
    }

    /// As ExportObject but for shared_ptr
    template<typename T>
    static auto ExportSharedObject(std::shared_ptr<T> obj) {
        ObjectLookupTableShared::AddObject(obj);
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T, v8pp::shared_ptr_traits>::import_external(isolate, obj);
    }

    /// Create JavaScript object which references externally created C++ class.
	/// It will not take ownership of the C++ pointer.
    template<typename T>
    static void ExportObjectRef(T* obj) {
        ObjectLookupTableRaw::AddObject(obj);
    }

    /// As ExportObjectRef but for shared_ptr
    template<typename T>
    static void ExportSharedObjectRef(std::shared_ptr<T> obj) {
        ObjectLookupTableShared::AddObject(obj);
    }

    /// Remove external reference from JavaScript
    template<typename T>
    static bool RemoveObjectRef(T* obj) {
        ObjectLookupTableRaw::RemoveObject(obj);
        auto isolate = v8::Isolate::GetCurrent();
        return v8pp::class_<T, v8pp::raw_ptr_traits>::unreference_external(isolate, obj);
    }
};