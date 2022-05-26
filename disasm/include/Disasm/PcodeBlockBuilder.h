#pragma once
#include <set>
#include "Core/Image/Image.h"
#include "DecoderPcode.h"

namespace sda::disasm
{
    // Builds p-code blocks
    class PcodeBlockBuilder
    {
        pcode::Graph* m_graph;
        Image* m_image;
        DecoderPcode* m_decoder;
        std::list<pcode::InstructionOffset> m_unvisitedOffsets;
        std::set<pcode::InstructionOffset> m_visitedOffsets;
        size_t m_curOrigInstrLength = 0;
    public:
        PcodeBlockBuilder(pcode::Graph* graph, Image* image, DecoderPcode* decoder);

        void start();

        void addUnvisitedOffset(pcode::InstructionOffset offset);

        // Callbacks for the builder
        struct Callbacks
        {
            // Called when an instruction is passed
            virtual void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) {}

            // Called when some warning is emitted
            virtual void onWarningEmitted(const std::string& message) {}
        };

        // Handle jump instructions
        struct StdCallbacks : Callbacks
        {
            PcodeBlockBuilder* m_builder;
            std::unique_ptr<Callbacks> m_nextCallbacks;
            std::set<pcode::Block*> m_affectedBlocks;

            // Called when an instruction is passed
            void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) override;

            // Called when some warning is emitted
            void onWarningEmitted(const std::string& message) override;

        protected:
            pcode::InstructionOffset getTargetOffset(const pcode::Instruction* instr);
        };

        // Set the callbacks for the builder
        std::unique_ptr<Callbacks> setCallbacks(std::unique_ptr<Callbacks> callbacks);

        // Get the callbacks for the builder
        Callbacks* getCallbacks() const;

    private:
        std::unique_ptr<Callbacks> m_callbacks;
    };
};