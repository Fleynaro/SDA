#pragma once
#include "SDA/Core/Platform/Platform.h"
#include "SDA/Core/Platform/RegisterRepository.h"
#include "SDA/Core/Platform/PcodeDecoder.h"
#include "SDA/Core/Platform/InstructionDecoder.h"
#include "SDA/Core/Platform/CallingConvention.h"

namespace sda::bind
{
    class PlatformBind
    {
        static auto GetPcodeDecoder(Platform* platform) {
            auto decoder = platform->getPcodeDecoder();
            ExportSharedObjectRef(decoder);
            return decoder;
        }

        static auto GetInstructionDecoder(Platform* platform) {
            auto decoder = platform->getInstructionDecoder();
            ExportSharedObjectRef(decoder);
            return decoder;
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Platform>(module);
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
            auto cl = NewClass<RegisterRepository, v8pp::shared_ptr_traits>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .method("getRegisterName", &RegisterRepository::getRegisterName)
                .method("getRegisterId", &RegisterRepository::getRegisterId)
                .method("getRegisterType", &RegisterRepository::getRegisterType)
                .method("getRegisterFlagName", &RegisterRepository::getRegisterFlagName)
                .method("getRegisterFlagIndex", &RegisterRepository::getRegisterFlagIndex);
            ObjectLookupTableShared<RegisterRepository>::Register(cl);
            module.class_("RegisterRepository", cl);
        }
    };

    class PcodeDecoderBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<PcodeDecoder, v8pp::shared_ptr_traits>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("instructionLength", &PcodeDecoder::getInstructionLength)
                .method("decode", &PcodeDecoder::decode);
            module.class_("PcodeDecoder", cl);
        }
    };

    class InstructionDecoderBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<InstructionDecoder, v8pp::shared_ptr_traits>(module);
            cl
                .auto_wrap_object_ptrs(true)
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
                auto cl = NewClass<CallingConvention::Storage>(module);
                cl
                    .auto_wrap_objects(true)
                    .var("useType", &CallingConvention::Storage::useType)
                    .var("registerId", &CallingConvention::Storage::registerId)
                    .var("offset", &CallingConvention::Storage::offset);
                module.class_("CallingConvention_Storage", cl);
            }

            {
                auto cl = NewClass<CallingConvention::StorageInfo>(module);
                cl
                    .auto_wrap_objects(true)
                    .var("type", &CallingConvention::StorageInfo::type)
                    .var("paramIdx", &CallingConvention::StorageInfo::paramIdx)
                    .var("isStoringFloat", &CallingConvention::StorageInfo::isStoringFloat);
                module.class_("CallingConvention_StorageInfo", cl);
            }

            {
                auto cl = NewClass<CallingConvention, v8pp::shared_ptr_traits>(module);
                cl
                    .auto_wrap_object_ptrs(true)
                    .property("name", &CallingConvention::getName)
                    .method("getStorages", &CallingConvention::getStorages);
                ObjectLookupTableShared<CallingConvention>::Register(cl);
                module.class_("CallingConvention", cl);
            }
        }
    };

    class CustomCallingConventionBind
    {
        static auto New(const CallingConvention::Map& storages) {
            auto cc = std::make_shared<CustomCallingConvention>(storages);
            ExportSharedObjectRef<CallingConvention>(cc);
            return cc;
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<CustomCallingConvention, v8pp::shared_ptr_traits>(module);
            cl
                .inherit<CallingConvention>()
                .static_method("new", &New);
            module.class_("CustomCallingConvention", cl);
        }
    };
};