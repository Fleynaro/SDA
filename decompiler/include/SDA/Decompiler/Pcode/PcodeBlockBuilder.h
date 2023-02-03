#pragma once
#include <set>
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/Platform/PcodeDecoder.h"

namespace sda::decompiler
{
    // Builds p-code blocks
    class PcodeBlockBuilder
    {
    public:
        class Provider {
        protected:
            pcode::Graph* m_graph;
        public:
            Provider(pcode::Graph* graph) : m_graph(graph) {}

            virtual void decode(Offset offset, size_t& origInstructionLength) = 0;

            virtual bool isOffsetValid(Offset offset) = 0;
        };

        class ImageProvider : public Provider {
            Image* m_image;
            PcodeDecoder* m_decoder;
        public:
            ImageProvider(pcode::Graph* graph, Image* image, PcodeDecoder* decoder)
                : Provider(graph), m_image(image), m_decoder(decoder)
            {}

            void decode(Offset offset, size_t& origInstructionLength) override;

            bool isOffsetValid(Offset offset) override;
        };

        class PcodeProvider : public Provider {
            std::map<pcode::InstructionOffset, pcode::Instruction> m_instructions;
        public:
            PcodeProvider(pcode::Graph* graph, const std::list<pcode::Instruction>& instructions);

            void decode(Offset offset, size_t& origInstructionLength) override;

            bool isOffsetValid(Offset offset) override;
        };

        pcode::Graph* m_graph;
        std::shared_ptr<Provider> m_provider;
        std::list<pcode::InstructionOffset> m_unvisitedOffsets;
        std::set<pcode::InstructionOffset> m_visitedOffsets;
        size_t m_curOrigInstrLength = 0;
    public:
        PcodeBlockBuilder(pcode::Graph* graph, Image* image, PcodeDecoder* decoder);

        PcodeBlockBuilder(pcode::Graph* graph, const std::list<pcode::Instruction>& instructions);

        PcodeBlockBuilder(pcode::Graph* graph, std::shared_ptr<Provider> provider);

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
            std::shared_ptr<PcodeBlockBuilder> m_builder;
            std::shared_ptr<Callbacks> m_nextCallbacks; // chain of responsibility pattern
            std::set<pcode::Block*> m_affectedBlocks;
        public:
            StdCallbacks(std::shared_ptr<PcodeBlockBuilder> builder, std::shared_ptr<Callbacks> nextCallbacks = nullptr);

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