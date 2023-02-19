#pragma once
#include <map>
#include <list>
#include <set>
#include <functional>
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

        bool contains(InstructionOffset offset, bool halfInterval = true) const;

        bool canBeJoinedWith(Block* block) const;

        void passDescendants(std::function<void(Block* block, bool& goNextBlocks)> callback);

        // Called when the block was changed
        void update();

    private:
        void updateEntryBlocks(bool& goNextBlocks);

        bool isEntryBlockInited() const;
    };
};