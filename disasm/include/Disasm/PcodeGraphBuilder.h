#pragma once
#include "PcodeBlockBuilder.h"

namespace sda::disasm
{
    // Builds p-code blocks and function graphs
    class PcodeGraphBuilder
    {
        pcode::Graph* m_graph;
        Image* m_image;
        PcodeBlockBuilder m_blockBuilder;
        
    public:
        PcodeGraphBuilder(pcode::Graph* graph, Image* image, DecoderPcode* decoder);

        PcodeBlockBuilder* getBlockBuilder();

        void start(const std::list<pcode::InstructionOffset>& startOffsets);

        // Callbacks for the graph builder
        class Callbacks
        {
        public:
            // Called when some warning is emitted
            virtual void onWarningEmitted(const std::string& message) {}
        };

        // Set the callbacks for the builder
        std::unique_ptr<Callbacks> setCallbacks(std::unique_ptr<Callbacks> callbacks);

        // Get the callbacks for the builder
        Callbacks* getCallbacks() const;

    private:
        std::unique_ptr<Callbacks> m_callbacks;
    };
};