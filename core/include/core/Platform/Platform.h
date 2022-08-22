#pragma once
#include <memory>
#include "RegisterRepository.h"
#include "PcodeDecoder.h"
#include "InstructionDecoder.h"
#include "CallingConvention.h"

namespace sda
{
    class Platform
    {
    public:
        virtual std::string getName() const = 0;

        virtual size_t getPointerSize() const = 0;

        virtual RegisterRepository* getRegisterRepository() const = 0;

        virtual const std::list<std::shared_ptr<CallingConvention>>& getCallingConventions() const = 0;

        virtual std::shared_ptr<PcodeDecoder> getPcodeDecoder() const = 0;

        virtual std::shared_ptr<InstructionDecoder> getInstructionDecoder() const = 0;
    };
};