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
                .method("getRegisterName", &RegisterRepository::getRegisterName)
                .method("getRegisterId", &RegisterRepository::getRegisterId)
                .method("getRegisterType", &RegisterRepository::getRegisterType)
                .method("getRegisterFlagName", &RegisterRepository::getRegisterFlagName)
                .method("getRegisterFlagIndex", &RegisterRepository::getRegisterFlagIndex);
            module.class_("RegisterRepository", cl);
        }
    };

    class PcodeDecoderBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<PcodeDecoder, v8pp::shared_ptr_traits> cl(module.isolate());
            cl
                .property("instructionLength", &PcodeDecoder::getInstructionLength)
                .method("decode", &PcodeDecoder::decode);
            module.class_("PcodeDecoder", cl);
        }
    };

    class InstructionDecoderBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<InstructionDecoder, v8pp::shared_ptr_traits> cl(module.isolate());
            cl
                .method("decode", &InstructionDecoder::decode);
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

    class CustomCallingConventionBind
    {
        static auto New() {
            return std::make_shared<CustomCallingConvention>();
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<CustomCallingConvention, v8pp::shared_ptr_traits> cl(module.isolate());
            cl
                .inherit<CallingConvention>()
                .static_method("new", &New);
            module.class_("CustomCallingConvention", cl);
        }
    };
};