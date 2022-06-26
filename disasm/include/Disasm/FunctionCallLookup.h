#pragma once
#include <list>
#include "PcodeBlockBuilder.h"

namespace sda::disasm
{
    // Lookup for function calls and gather function offsets
    class FunctionCallLookupCallbacks : public PcodeBlockBuilder::StdCallbacks
    {
        std::list<std::pair<pcode::InstructionOffset, pcode::InstructionOffset>>* m_constFunctionOffsets; // from -> to
    public:
        FunctionCallLookupCallbacks(
            std::list<std::pair<pcode::InstructionOffset, pcode::InstructionOffset>>* constFunctionOffsets,
            PcodeBlockBuilder* builder,
            std::unique_ptr<Callbacks> nextCallbacks = nullptr);

        // Called when an instruction is passed
        void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) override;
    };
};