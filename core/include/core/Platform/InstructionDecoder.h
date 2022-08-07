#pragma once
#include <vector>
#include "Instruction.h"

namespace sda
{
    // This decoder is used to decode instructions for rendering
    class InstructionDecoder
    {
    public:
        virtual void decode(const std::vector<uint8_t>& data) = 0;

        const Instruction* getDecodedInstruction() const;
        
    protected:
        Instruction m_decodedInstruction;
    };
};