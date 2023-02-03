#pragma once
#include "PcodeBlockBuilder.h"

namespace sda::decompiler
{
    // Builds p-code blocks and function graphs
    class PcodeGraphBuilder
    {
        pcode::Graph* m_graph;
        Image* m_image;
        std::shared_ptr<PcodeBlockBuilder> m_blockBuilder;
        
    public:
        PcodeGraphBuilder(pcode::Graph* graph, std::shared_ptr<PcodeBlockBuilder> blockBuilder);

        void start(const std::list<pcode::InstructionOffset>& startOffsets, bool fromEntryPoints = false);

        // Callbacks for the graph builder
        class Callbacks
        {
        public:
            // Called when some warning is emitted
            virtual void onWarningEmitted(const std::string& message) {}
        };

        // Set the callbacks for the builder
        void setCallbacks(std::shared_ptr<Callbacks> callbacks);

        // Get the callbacks for the builder
        std::shared_ptr<Callbacks> getCallbacks() const;

    private:
        std::shared_ptr<Callbacks> m_callbacks;

        void createFunctionGraph(pcode::InstructionOffset offset);
    };
};