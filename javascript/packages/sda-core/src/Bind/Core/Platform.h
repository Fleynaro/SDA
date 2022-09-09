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
            {
                // TODO: will see https://github.com/pmed/v8pp/issues/31
                v8pp::class_<CallingConvention::Storage> cl(module.isolate());
                cl
                    .auto_wrap_objects(true)
                    .var("useType", &CallingConvention::Storage::useType)
                    .var("registerId", &CallingConvention::Storage::registerId)
                    .var("offset", &CallingConvention::Storage::offset);
                module.class_("CallingConvention_Storage", cl);
            }

            {
                v8pp::class_<CallingConvention::StorageInfo> cl(module.isolate());
                cl
                    .auto_wrap_objects(true)
                    .var("type", &CallingConvention::StorageInfo::type)
                    .var("paramIdx", &CallingConvention::StorageInfo::paramIdx)
                    .var("isStoringFloat", &CallingConvention::StorageInfo::isStoringFloat);
                module.class_("CallingConvention_StorageInfo", cl);
            }

            {
                v8pp::class_<CallingConvention, v8pp::shared_ptr_traits> cl(module.isolate());
                cl
                    .property("name", &CallingConvention::getName)
                    .method("getStorages", &CallingConvention::getStorages);
                module.class_("CallingConvention", cl);
            }
        }
    };

    class CustomCallingConventionBind
    {
        static auto New(const CallingConvention::Map& storages) {
            return std::make_shared<CustomCallingConvention>(storages);
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