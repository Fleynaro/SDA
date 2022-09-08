#pragma once
#include "Platform/X86/Platform.h"

namespace sda::bind
{
    class PlatformX86Bind
    {
        static auto Take(PlatformX86* platform) {
            RemoveObjectRef(platform);
            return ExportObject(platform);
        }

        static auto New(bool is64Version) {
            auto platform = new PlatformX86(is64Version);
            ExportObjectRef(platform->getRegisterRepository());
            for (auto cc : platform->getCallingConventions())
                ExportObject(cc);
            return ExportObjectRef(platform);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<PlatformX86> cl(module.isolate());
            cl
                .inherit<Platform>()
                .method("take", &Take)
                .static_method("New", &New);
            module.class_("PlatformX86", cl);
        }
    };
};