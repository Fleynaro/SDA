#include "Bind/Init.h"
#include "Bind/Platform/PlatformX86.h"
#include <iostream>

using namespace sda::bind;

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
    InitModule(module, {
        PlatformX86Bind::Init
    });
}

NODE_MODULE(platform_x86, Init)