#pragma once
#include <map>
#include "SDA/Core/Platform/PcodeDecoder.h"

namespace sda {
    class Image;
};

namespace sda::pcode
{
    class InstructionProvider {
    public:
        virtual void decode(Offset offset, std::list<pcode::Instruction>& instructions, size_t& origInstrLength) = 0;

        virtual bool isOffsetValid(Offset offset) = 0;
    };

    class ListInstructionProvider : public InstructionProvider {
        std::map<pcode::InstructionOffset, pcode::Instruction> m_instructions;
    public:
        ListInstructionProvider(const std::list<pcode::Instruction>& instructions);

        void decode(Offset offset, std::list<pcode::Instruction>& instructions, size_t& origInstrLength) override;

        bool isOffsetValid(Offset offset) override;
    };

    // TODO: This class could be moved to Image.h as p-code module should be independent from core module
    class ImageInstructionProvider : public InstructionProvider {
        Image* m_image;
        std::shared_ptr<PcodeDecoder> m_decoder;
    public:
        ImageInstructionProvider(Image* image);

        void decode(Offset offset, std::list<pcode::Instruction>& instructions, size_t& origInstrLength) override;

        bool isOffsetValid(Offset offset) override;
    };
};