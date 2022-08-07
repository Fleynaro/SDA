#pragma once
#include "Core/Platform/InstructionDecoder.h"
#include <Zydis/Zydis.h>

namespace sda::platform
{
    class InstructionDecoderX86 : public InstructionDecoder
    {
        ZydisDecoder* m_decoder;
        ZydisFormatter m_formatter;
    public:
        InstructionDecoderX86(ZydisDecoder* decoder);

        void decode(const std::vector<uint8_t>& data) override;
    };
};
