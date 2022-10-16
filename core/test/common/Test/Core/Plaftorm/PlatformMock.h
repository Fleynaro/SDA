#pragma once
#include <gmock/gmock.h>
#include "SDA/Core/Platform/Platform.h"

namespace sda::test
{
    class PlatformMock : public Platform
    {
    public:
        MOCK_METHOD(std::string, getName, (), (const, override));

        MOCK_METHOD(size_t, getPointerSize, (), (const, override));

        MOCK_METHOD(RegisterRepository*, getRegisterRepository, (), (const, override));

        MOCK_METHOD(const std::list<std::shared_ptr<CallingConvention>>&, getCallingConventions, (), (const, override));

        MOCK_METHOD(std::shared_ptr<PcodeDecoder>, getPcodeDecoder, (), (const, override));
        
        MOCK_METHOD(std::shared_ptr<InstructionDecoder>, getInstructionDecoder, (), (const, override));
    };
};