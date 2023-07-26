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
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Platform>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &Platform::getName)
                .property("pointerSize", &Platform::getPointerSize)
                .property("registerRepository", &Platform::getRegisterRepository)
                .property("callingConventions", &Platform::getCallingConventions)
                .method("getPcodeDecoder", &Platform::getPcodeDecoder)
                .method("getInstructionDecoder", &Platform::getInstructionDecoder);
            ObjectLookupTableRaw::Register(cl);
            RegisterClassName(cl, "Platform");
            module.class_("Platform", cl);
        }
    };

    class PlatformMockBind
    {
        class PlatformMock : public Platform
        {
            std::list<std::shared_ptr<CallingConvention>> m_callingConventions;
        public:
            std::string getName() const override {
                return "mock";
            }

            size_t getPointerSize() const override {
                return 8;
            }

            std::shared_ptr<RegisterRepository> getRegisterRepository() const override {
                return nullptr;
            }

            const std::list<std::shared_ptr<CallingConvention>>& getCallingConventions() const override {
                return m_callingConventions;
            }

            std::shared_ptr<PcodeDecoder> getPcodeDecoder() const override {
                return nullptr;
            }

            std::shared_ptr<InstructionDecoder> getInstructionDecoder() const override {
                return nullptr;
            }
        };

        static auto New() {
            auto platform = new PlatformMock();
            return ExportObject(platform);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<PlatformMock>(module);
            cl
                .inherit<Platform>()
                .static_method("New", &New);
            module.class_("PlatformMock", cl);
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
            ObjectLookupTableShared::Register(cl);
            RegisterClassName(cl, "RegisterRepository");
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
            ObjectLookupTableShared::Register(cl);
            RegisterClassName(cl, "PcodeDecoder");
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
            ObjectLookupTableShared::Register(cl);
            RegisterClassName(cl, "InstructionDecoder");
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
                ObjectLookupTableShared::Register(cl);
                RegisterClassName(cl, "CallingConvention");
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
            auto cl = NewClass<CustomCallingConvention, v8pp::shared_ptr_traits>(module);
            cl
                .inherit<CallingConvention>()
                .static_method("new", &New);
            module.class_("CustomCallingConvention", cl);
        }
    };
};