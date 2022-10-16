#pragma once
#include "SDA/Core/Platform/Platform.h"
#include "SDA/Core/Platform/RegisterRepository.h"
#include <Zydis/Zydis.h>

namespace sda
{
    class PlatformX86 : public Platform
    {
        bool m_is64Version;
        std::unique_ptr<RegisterRepository> m_regRepo;
        std::list<std::shared_ptr<CallingConvention>> m_callingConventions;
    public:
        PlatformX86(bool is64Version);

        std::string getName() const override;

        size_t getPointerSize() const override;

        RegisterRepository* getRegisterRepository() const override;

        const std::list<std::shared_ptr<CallingConvention>>& getCallingConventions() const override;

        std::shared_ptr<PcodeDecoder> getPcodeDecoder() const override;

        std::shared_ptr<InstructionDecoder> getInstructionDecoder() const override;
    };
};