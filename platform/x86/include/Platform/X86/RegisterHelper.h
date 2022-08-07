#pragma once
#include "Core/Platform/RegisterHelper.h"
#include <Zydis/Zydis.h>

namespace sda::platform
{
    class RegisterHelperX86 : public RegisterHelper
    {
    public:
        std::string getRegisterName(size_t regId) const override;

        size_t getRegisterId(const std::string& regName) const override;

        Register::Type getRegisterType(size_t regId) const override;

        std::string getRegisterFlagName(size_t flagMask) const override;

        size_t getRegisterFlagIndex(const std::string& flagName) const override;

        // this transform allows to make the same id for different parts of the same register (e.g. rax/eax/ax/al has the same id)
        size_t transformZydisRegId(ZydisRegister regId) const;
    };
};
