#pragma once
#include "SDA/Core/Platform/InstructionDecoder.h"
#include <Zydis/Zydis.h>

namespace sda::platform
{
    class InstructionDecoderX86 : public InstructionDecoder
    {
        std::unique_ptr<ZydisDecoder> m_decoder;
        std::unique_ptr<ZydisFormatter> m_formatter;
    public:
        InstructionDecoderX86(std::unique_ptr<ZydisDecoder> decoder, std::unique_ptr<ZydisFormatter> formatter);

        void decode(const std::vector<uint8_t>& data, bool tokenize = true) override;
    };
};
