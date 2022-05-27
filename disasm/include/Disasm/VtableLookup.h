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
        std::list<VTable> m_vtables;
        Image* m_image;
    public:
        VtableLookupCallbacks(Image* image, PcodeBlockBuilder* builder);

        const std::list<VTable>& getVtables() const;

        // Called when an instruction is passed
        void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) override;
    };
};