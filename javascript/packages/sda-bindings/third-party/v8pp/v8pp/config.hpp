#ifndef V8PP_CONFIG_HPP_INCLUDED
#define V8PP_CONFIG_HPP_INCLUDED

/// v8pp library version
#define V8PP_VERSION "2.0.0"

/// v8::Isolate data slot number, used in v8pp for shared data
#if !defined(V8PP_ISOLATE_DATA_SLOT)
#define V8PP_ISOLATE_DATA_SLOT 1
#endif

/// v8pp plugin initialization procedure name
#if !defined(V8PP_PLUGIN_INIT_PROC_NAME)
#define V8PP_PLUGIN_INIT_PROC_NAME v8pp_module_init
#endif

/// v8pp plugin filename suffix
#if !defined(V8PP_PLUGIN_SUFFIX)
#define V8PP_PLUGIN_SUFFIX ".dll"
#endif

#if defined(_MSC_VER)
#define V8PP_EXPORT __declspec(dllexport)
#define V8PP_IMPORT __declspec(dllimport)
#elif __GNUC__ >= 4
#define V8PP_EXPORT __attribute__((__visibility__("default")))
#define V8PP_IMPORT V8PP_EXPORT
#else
#define V8PP_EXPORT
#define V8PP_IMPORT
#endif

#define V8PP_PLUGIN_INIT(isolate) extern "C" V8PP_EXPORT \
v8::Local<v8::Value> V8PP_PLUGIN_INIT_PROC_NAME(isolate)

#ifndef V8PP_HEADER_ONLY
#define V8PP_HEADER_ONLY 1
#endif

#if V8PP_HEADER_ONLY
#define V8PP_IMPL inline
#else
#define V8PP_IMPL
#endif

#if !defined(V8PP_USE_STD_STRING_VIEW)
#define V8PP_USE_STD_STRING_VIEW 0
#endif

#define V8PP_STRINGIZE(s)  V8PP_STRINGIZE0(s)
#define V8PP_STRINGIZE0(s) #s

#endif // V8PP_CONFIG_HPP_INCLUDED
