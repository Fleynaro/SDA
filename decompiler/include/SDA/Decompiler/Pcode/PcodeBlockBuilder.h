#pragma once
#include <set>
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/Platform/PcodeDecoder.h"

namespace sda::decompiler
{
    // Builds p-code blocks
    class PcodeBlockBuilder
    {
        pcode::Graph* m_graph;
        Image* m_image;
        PcodeDecoder* m_decoder;
        std::list<pcode::InstructionOffset> m_unvisitedOffsets;
        std::set<pcode::InstructionOffset> m_visitedOffsets;
        size_t m_curOrigInstrLength = 0;
    public:
        PcodeBlockBuilder(pcode::Graph* graph, Image* image, PcodeDecoder* decoder);

        void start();

        void addUnvisitedOffset(pcode::InstructionOffset offset);

        // Callbacks for the block builder
        class Callbacks
        {
        public:
            // Called when an instruction is passed
            virtual void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) {}

            // Called when some warning is emitted
            virtual void onWarningEmitted(const std::string& message) {}
        };

        // Handle jump instructions
        class StdCallbacks : public Callbacks
        {
        protected:
            PcodeBlockBuilder* m_builder;
            std::shared_ptr<Callbacks> m_nextCallbacks; // chain of responsibility pattern
            std::set<pcode::Block*> m_affectedBlocks;
        public:
            StdCallbacks(PcodeBlockBuilder* builder, std::shared_ptr<Callbacks> nextCallbacks = nullptr);

            const std::set<pcode::Block*>& getAffectedBlocks() const;

            // Called when an instruction is passed
            void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) override;

            // Called when some warning is emitted
            void onWarningEmitted(const std::string& message) override;

        protected:
            pcode::InstructionOffset getTargetOffset(const pcode::Instruction* instr);
        };

        // Set the callbacks for the builder
        void setCallbacks(std::shared_ptr<Callbacks> callbacks);

        // Get the callbacks for the builder
        std::shared_ptr<Callbacks> getCallbacks() const;

    private:
        std::shared_ptr<Callbacks> m_callbacks;
    };
};