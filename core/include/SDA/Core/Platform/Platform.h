#pragma once
#include <string>
#include <list>
#include <memory>
#include "SDA/Core/Utils/Wrapping.h"

namespace sda
{
    class RegisterRepository;
    class CallingConvention;
    class PcodeDecoder;
    class InstructionDecoder;

    class Platform : public utils::IWrappable
    {
    public:
        virtual std::string getName() const = 0;

        virtual size_t getPointerSize() const = 0;

        virtual std::shared_ptr<RegisterRepository> getRegisterRepository() const = 0;

        virtual const std::list<std::shared_ptr<CallingConvention>>& getCallingConventions() const = 0;

        virtual std::shared_ptr<PcodeDecoder> getPcodeDecoder() const = 0;

        virtual std::shared_ptr<InstructionDecoder> getInstructionDecoder() const = 0;
    };
};