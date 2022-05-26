#pragma once
#include "PcodeBlockBuilder.h"

namespace sda::disasm
{
    // Builds p-code blocks and function graphs (todo: dont search for vtable!)
    class PcodeGraphBuilder
    {
        // Block builder for calls and vtables
        struct BlockBuilderCallbacks : PcodeBlockBuilder::StdCallbacks
        {
            struct VTable {
                Offset m_offset;
                std::list<Offset> m_funcOffsets;
            };
            std::list<VTable> m_vtables;
            // todo: vtable part move to another class as vtable founder. rename BlockBuilderCallbacks -> FunctionCallCallbacks

            std::list<std::pair<pcode::InstructionOffset, pcode::InstructionOffset>> m_constFunctionOffsets; // from -> to
            PcodeGraphBuilder* m_graphBuilder;
            Image* m_image;

            // Called when an instruction is passed
            void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) override;
        };

        pcode::Graph* m_graph;
        Image* m_image;
        DecoderPcode* m_decoder;
        
    public:
        PcodeGraphBuilder(pcode::Graph* graph, Image* image, DecoderPcode* decoder);

        void start(
            const std::list<pcode::InstructionOffset>& startOffsets,
            std::unique_ptr<PcodeBlockBuilder::Callbacks> nextCallbacks = nullptr);

        // Callbacks for the builder
        struct Callbacks
        {
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