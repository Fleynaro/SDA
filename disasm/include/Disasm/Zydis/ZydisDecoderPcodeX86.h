#pragma once
#include "Disasm/DecoderPcode.h"
#include <Zydis/Zydis.h>

namespace sda::disasm
{
    class ZydisDecoderPcodeX86 : public DecoderPcode
    {
        ZydisDecoder* m_decoder;
        ZydisDecodedInstruction m_curInstr;
        ZydisDecodedOperand m_curOperands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];
        Offset m_curOffset = 0;
        size_t m_curInstrIndex = 0;
    public:
        std::string m_curInstrView; // for debug purposes

        ZydisDecoderPcodeX86(ZydisDecoder* decoder);

        void decode(Offset offset, const std::vector<uint8_t>& data) override;

        size_t getInstructionLength() const override;

    private:
        void generatePcodeInstructions();

        int getFirstExplicitOperandsCount() const;

        std::shared_ptr<pcode::Varnode> getJumpOffsetByOperand(const ZydisDecodedOperand& operand);

		size_t getJumpOffset(int jmpOffset) const;

        struct VectorOperationGeneratorInfo {
            int size = 0x0;
            int maxSize = 0x0;
            pcode::InstructionId instrId1 = pcode::InstructionId::NONE;
            pcode::InstructionId instrId2 = pcode::InstructionId::NONE;
            bool isNegate = false;
            bool isOperationWithSingleOperand = false;
            int shuffOp1[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
            int shuffOp2[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        };
		void generateVectorOperation(const VectorOperationGeneratorInfo& info);

		std::shared_ptr<pcode::Varnode> generateGenericOperation(
            pcode::InstructionId instrId,
            std::shared_ptr<pcode::Varnode> varnodeInput0,
            std::shared_ptr<pcode::Varnode> varnodeInput1,
            std::shared_ptr<pcode::Varnode> memLocVarnode = nullptr,
            bool isFictitious = false,
            int size = 0x0,
            int offset = 0x0);

		pcode::Instruction* generateInstruction(
            pcode::InstructionId id,
            std::shared_ptr<pcode::Varnode> input0,
            std::shared_ptr<pcode::Varnode> input1 = nullptr,
            std::shared_ptr<pcode::Varnode> output = nullptr,
            bool zext = true);

		std::shared_ptr<pcode::Varnode> getOperandVarnode(
            const ZydisDecodedOperand& operand,
            int size,
            std::shared_ptr<pcode::Varnode>* memLocVarnode = nullptr,
            bool isMemLocLoaded = true,
            int memLocExprSize = 0x8);

		std::shared_ptr<pcode::Varnode> getOperandVarnode(
            const ZydisDecodedOperand& operand,
            int size,
            int offset,
            std::shared_ptr<pcode::Varnode>* memLocVarnode = nullptr,
            bool isMemLocLoaded = true,
            int memLocExprSize = 0x8);

        enum class FlagCond {
            NONE,
            Z,
            NZ,
            C,
            NC,
            S,
            NS,
            O,
            NO,
            P,
            NP,
            L,
            LE,
            NL,
            NLE,
            A,
            NA
        };
		std::shared_ptr<pcode::Varnode> getConditionFlagVarnode(FlagCond flagCond);

        std::shared_ptr<pcode::RegisterVarnode> getRegisterVarnode(ZydisRegister regId, size_t size, int offset = 0x0) const;

        std::shared_ptr<pcode::RegisterVarnode> getRegisterVarnode(ZydisAccessedFlagsMask flagMask) const;

        std::shared_ptr<pcode::SymbolVarnode> getSymbolVarnode(size_t size) const;

        std::shared_ptr<pcode::ConstantVarnode> getConstantVarnode(size_t value, size_t size, bool isAddress = false) const;
    };
};