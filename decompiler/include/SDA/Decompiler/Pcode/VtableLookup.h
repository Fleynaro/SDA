#pragma once
#include "PcodeBlockBuilder.h"

namespace sda::decompiler
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
        VtableLookupCallbacks(Image* image, std::shared_ptr<PcodeBlockBuilder> builder, std::unique_ptr<Callbacks> nextCallbacks = nullptr);

        const std::list<VTable>& getVtables() const;

        // Called when an instruction is passed
        void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) override;
    };
};