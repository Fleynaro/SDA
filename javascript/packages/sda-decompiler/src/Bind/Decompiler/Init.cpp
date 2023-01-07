#include "Bind/Init.h"
#include "Bind/Core/Shared/Converter.h"
#include "Bind/Decompiler/Pcode.h"

using namespace sda::bind;

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
    InitModule(module, {
        // p-code
        PcodeGraphBuilderBind::Init,
    });
}

NODE_MODULE(decompiler, Init)