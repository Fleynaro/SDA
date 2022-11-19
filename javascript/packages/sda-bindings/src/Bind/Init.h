#pragma once
#include <node_version.h>

// https://github.com/electron/electron/blob/9-x-y/patches/node/enable_31_bit_smis_on_64bit_arch_and_ptr_compression.patch
#if NODE_MODULE_VERSION >= 80 // Electron 9
#if !defined(_WIN32) || defined(_WIN64)
#define V8_COMPRESS_POINTERS
#endif
#define V8_31BIT_SMIS_ON_64BIT_ARCH
#endif

// https://github.com/electron/electron/blob/11-x-y/patches/node/fix_add_v8_enable_reverse_jsargs_defines_in_common_gypi.patch
#if NODE_MODULE_VERSION >= 85 // Electron 11
#define V8_REVERSE_JSARGS
#endif

// https://github.com/nodejs/node/blob/v16.9.1/common.gypi#L361
#if NODE_MODULE_VERSION >= 99 // Electron 16
#if !defined(_WIN32) || defined(_WIN64)
#define V8_COMPRESS_POINTERS_IN_ISOLATE_CAGE
#endif
#endif

#include <node.h>

#include <v8pp/json.hpp>
#include <v8pp/module.hpp>
#include <v8pp/object.hpp>

#include "Convert.h"
#include "ObjectLookupTable.h"
#include "ObjectExport.h"
#include "Callback.h"

namespace sda::bind
{
    template<typename T, typename Traits = v8pp::raw_ptr_traits>
    v8pp::class_<T, Traits> NewClass(
        v8pp::module& module,
        std::function<void(T* obj)> deleter = nullptr)
    {
        using ClassType = v8pp::class_<T, Traits>;
        // Create destructor that called when object is garbage collected (removed from JS)
        auto dtor = ClassType::dtor_function(
            [deleter](v8::Isolate* isolate, ClassType::object_pointer_type const& obj) {
                if constexpr (std::is_same_v<ClassType::object_pointer_type, T*>) {
                    ObjectLookupTableRaw::RemoveObject(obj);
                    if (deleter)
                        deleter(obj);
                } else if constexpr (std::is_same_v<ClassType::object_pointer_type, std::shared_ptr<T>>) {
                    ObjectLookupTableShared::RemoveObject(obj);
                    if (deleter)
                        deleter(obj.get());
                }
                ClassType::object_destroy(isolate, obj);
            });
        ClassType cl(module.isolate(), dtor);
        return cl;
    }
    
    void InitModule(v8::Local<v8::Object> module, std::list<std::function<void(v8pp::module&)>> inits);
};