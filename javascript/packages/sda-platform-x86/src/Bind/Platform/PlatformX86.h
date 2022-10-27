#pragma once
#include "SDA/Platform/X86/Platform.h"
#include "SDA/Core/Platform/PcodeDecoder.h"
#include "SDA/Core/Platform/InstructionDecoder.h"
#include "SDA/Core/Platform/CallingConvention.h"

namespace sda::bind
{
    class PlatformX86Bind
    {
        static auto New(bool is64Version) {
            auto platform = new PlatformX86(is64Version);
            ExportSharedObject(platform->getRegisterRepository());
            return ExportObject(platform);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<PlatformX86>(module);
            cl
                .inherit<Platform>()
                .static_method("New", &New);
            module.class_("PlatformX86", cl);
        }
    };
};