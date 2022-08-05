#pragma once
#include <list>
#include "PcodeBlockBuilder.h"

namespace sda::decompiler
{
    // Lookup for function calls and gather function offsets
    class FunctionCallLookupCallbacks : public PcodeBlockBuilder::StdCallbacks
    {
        std::list<std::pair<pcode::InstructionOffset, pcode::InstructionOffset>> m_constFunctionOffsets; // from -> to
    public:
        using PcodeBlockBuilder::StdCallbacks::StdCallbacks;

        const std::list<std::pair<pcode::InstructionOffset, pcode::InstructionOffset>>& getConstFunctionOffsets() const;

        // Called when an instruction is passed
        void onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) override;
    };
};