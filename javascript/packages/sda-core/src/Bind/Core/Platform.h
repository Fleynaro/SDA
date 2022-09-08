#pragma once
#include "Core/Platform/Platform.h"
#include "Core/Platform/RegisterRepository.h"
#include "Core/Platform/PcodeDecoder.h"
#include "Core/Platform/InstructionDecoder.h"
#include "Core/Platform/CallingConvention.h"

namespace sda::bind
{
    class PlatformBind
    {
        static auto GetPcodeDecoder(Platform* platform) {
            return ExportObject(platform->getPcodeDecoder());
        }

        static auto GetInstructionDecoder(Platform* platform) {
            return ExportObject(platform->getInstructionDecoder());
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<Platform> cl(module.isolate());
            cl
                .property("name", &Platform::getName)
                .property("pointerSize", &Platform::getPointerSize)
                .property("registerRepository", &Platform::getRegisterRepository)
                .property("callingConventions", &Platform::getCallingConventions)
                .method("getPcodeDecoder", &GetPcodeDecoder)
                .method("getInstructionDecoder", &GetInstructionDecoder);
            module.class_("Platform", cl);
        }
    };

    class RegisterRepositoryBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<RegisterRepository> cl(module.isolate());
            cl
                .method<std::string, size_t>("getRegisterName", &RegisterRepository::getRegisterName)
                .method<size_t, const std::string&>("getRegisterId", &RegisterRepository::getRegisterId)
                .method<Register::Type, size_t>("getRegisterType", &RegisterRepository::getRegisterType)
                .method<std::string, size_t>("getRegisterFlagName", &RegisterRepository::getRegisterFlagName)
                .method<size_t, const std::string&>("getRegisterFlagIndex", &RegisterRepository::getRegisterFlagIndex);
            module.class_("RegisterRepository", cl);
        }
    };

    class PcodeDecoderBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<PcodeDecoder, v8pp::shared_ptr_traits> cl(module.isolate());
            module.class_("PcodeDecoder", cl);
        }
    };

    class InstructionDecoderBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<InstructionDecoder, v8pp::shared_ptr_traits> cl(module.isolate());
            module.class_("InstructionDecoder", cl);
        }
    };

    class CallingConventionBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<CallingConvention, v8pp::shared_ptr_traits> cl(module.isolate());
            cl
                .property("name", &CallingConvention::getName);
            module.class_("CallingConvention", cl);
        }
    };
};