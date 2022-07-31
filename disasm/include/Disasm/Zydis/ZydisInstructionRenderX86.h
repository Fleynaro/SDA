#pragma once
#include "Disasm/InstructionRender.h"
#include <Zydis/Zydis.h>

namespace sda::disasm
{
    class ZydisDecoderRenderX86 : public DecoderRender
    {
        ZydisDecoder* m_decoder;
        ZydisFormatter m_formatter;
    public:
        ZydisDecoderRenderX86(ZydisDecoder* decoder);

        void decode(const std::vector<uint8_t>& data) override;
    };
};
