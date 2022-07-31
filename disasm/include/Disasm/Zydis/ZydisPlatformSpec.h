#pragma once
#include "Disasm/InstructionRender.h"
#include "Core/Pcode/PcodePlatformSpec.h"
#include <Zydis/Zydis.h>

namespace sda::disasm
{
    class ZydisPlatformSpec : public pcode::PlatformSpec
    {
    public:
        std::string getRegisterName(size_t regId) const override;

        size_t getRegisterId(const std::string& regName) const override;

        pcode::RegisterVarnode::Type getRegisterType(size_t regId) const override;

        std::string getRegisterFlagName(size_t flagMask) const override;

        size_t getRegisterFlagIndex(const std::string& flagName) const override;

        // this transform allows to make the same id for different parts of the same register (e.g. rax/eax/ax/al has the same id)
        size_t transformZydisRegId(ZydisRegister regId) const;
    };
};
