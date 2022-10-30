#pragma once
#include <vector>
#include "Instruction.h"
#include "SDA/Core/Utils/Wrapping.h"

namespace sda
{
    // This decoder is used to decode instructions for printing
    class InstructionDecoder : public utils::IWrappable
    {
    public:
        virtual void decode(const std::vector<uint8_t>& data) = 0;

        const Instruction* getDecodedInstruction() const;
        
    protected:
        Instruction m_decodedInstruction;
    };
};