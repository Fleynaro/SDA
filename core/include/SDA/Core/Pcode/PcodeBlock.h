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
        Block* m_entryBlock = nullptr;
        size_t m_level = 0;
    public:
        Block() = default;

        Block(Graph* graph, InstructionOffset minOffset);

        std::string getName() const;

        Graph* getGraph();

        std::map<InstructionOffset, const Instruction*>& getInstructions();

        const Instruction* getLastInstruction() const;

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

        Block* getEntryBlock() const;

        bool isEntryBlock() const;

        size_t getLevel() const;

        bool contains(InstructionOffset offset, bool halfInterval = true) const;

        // This method is used to check if the EXISTING link to the block is a loop
        bool hasLoopWith(Block* block) const;

        bool canReach(Block* blockToReach) const;

        bool canBeJoinedWith(Block* block) const;

        // Called when the block was changed
        void update();

    private:
        void update(void (Block::*updateMethod)(bool& goNextBlocks));

        void updateLevels(bool& goNextBlocks);

        void updateEntryBlocks(bool& goNextBlocks);

        bool isLevelInited() const;

        bool isEntryBlockInited() const;
    };
};