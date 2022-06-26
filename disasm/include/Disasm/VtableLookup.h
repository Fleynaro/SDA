#pragma once
#include "PcodeBlockBuilder.h"

namespace sda::disasm
{
    // Lookup for vtables and gather them with virtual function offsets
    class VtableLookupCallbacks : public PcodeBlockBuilder::StdCallbacks
    {
        struct VTable {
            Offset m_offset;
            std::list<Offset> m_virtFuncOffsets;
        };
        std::list<VTable>* m_vtables;
        Image* m_image;
    public:
        VtableLookupCallbacks(std::list<VTable>* vtables, Image* image, PcodeBlockBuilder* builder, std::unique_ptr<Callbacks> nextCallbacks = nullptr);

        // Called when an instruction is passed
        void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) override;
    };
};