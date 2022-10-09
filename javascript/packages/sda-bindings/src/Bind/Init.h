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
#include "Call.h"

namespace sda::bind
{
    void InitModule(v8::Local<v8::Object> module, std::list<std::function<void(v8pp::module&)>> inits);

    template<typename T, typename Traits = v8pp::raw_ptr_traits>
    v8pp::class_<T, Traits> NewClass(v8pp::module& module, bool registerObjectLookupTable = true) {
        v8pp::class_<T, Traits> cl(module.isolate());
        if (registerObjectLookupTable)
            ObjectLookupTable<T>::Register(cl);
        return cl;
    }
};