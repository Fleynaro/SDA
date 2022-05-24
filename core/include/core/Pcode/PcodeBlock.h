#pragma once
#include <list>
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Block
    {
        std::list<const Instruction*> m_instructions;
        Block* m_nearNextBlock;
        Block* m_farNextBlock;
        std::list<Block*> m_referencedBlocks;
        InstructionOffset m_minOffset;
        InstructionOffset m_maxOffset;
    public:
        Block(InstructionOffset minOffset);

        std::list<const Instruction*>& getInstructions();

        void setNearNextBlock(Block* nearNextBlock);

        Block* getNearNextBlock() const;

        void setFarNextBlock(Block* farNextBlock);

        Block* getFarNextBlock() const;
        
        const std::list<Block*>& getReferencedBlocks() const;

        InstructionOffset getMinOffset() const;

        void setMaxOffset(InstructionOffset maxOffset);

        InstructionOffset getMaxOffset() const;

        bool contains(InstructionOffset offset, bool halfInterval = true) const;
    };
};