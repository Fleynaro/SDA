#pragma once
#include "Register.h"
#include "SDA/Core/Utils/Wrapping.h"

namespace sda
{
    // Here are some functions depending on the platform to get info about registers.
    class RegisterRepository : public utils::IWrappable
    {
    public:
        virtual std::string getRegisterName(size_t regId) const = 0;

        virtual size_t getRegisterId(const std::string& regName) const = 0;

        virtual Register::Type getRegisterType(size_t regId) const = 0;

        virtual std::string getRegisterFlagName(size_t flagMask) const = 0;

        virtual size_t getRegisterFlagIndex(const std::string& flagName) const = 0;
    };
};