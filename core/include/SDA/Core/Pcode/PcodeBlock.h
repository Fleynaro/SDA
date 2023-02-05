#pragma once
#include <map>
#include <list>
#include <set>
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Graph;
    class FunctionGraph;

    class Block
    {
        Graph* m_graph;
        std::map<InstructionOffset, const Instruction*> m_instructions;
        Block* m_nearNextBlock = nullptr;
        Block* m_farNextBlock = nullptr;
        std::list<Block*> m_referencedBlocks;
        InstructionOffset m_minOffset = 0;
        InstructionOffset m_maxOffset = 0;
        FunctionGraph* m_functionGraph = nullptr;
        size_t m_level = 0;
    public:
        Block();

        Block(Graph* graph, InstructionOffset minOffset);

        std::string getName() const;

        std::map<InstructionOffset, const Instruction*>& getInstructions();

        void setNearNextBlock(Block* nearNextBlock);

        Block* getNearNextBlock() const;

        void setFarNextBlock(Block* farNextBlock);

        Block* getFarNextBlock() const;

        std::list<Block*> getNextBlocks() const;
        
        const std::list<Block*>& getReferencedBlocks() const;

        InstructionOffset getMinOffset() const;

        void setMaxOffset(InstructionOffset maxOffset);

        InstructionOffset getMaxOffset() const;

        FunctionGraph* getFunctionGraph() const;

        size_t getLevel() const;

        bool contains(InstructionOffset offset, bool halfInterval = true) const;

        bool isInited() const;

        bool hasLoop() const;

        // Called when the block was changed
        void update();
    };
};