#pragma once
#include "DecPCodeAbstractDecoder.h"
#include <Zycore/Format.h>
#include <Zycore/LibC.h>
#include <Zydis/Zydis.h>

namespace CE::Decompiler::PCode
{
	class DecoderX86 : public AbstractDecoder
	{
		ZydisDecoder m_decoder;
		RegisterFactoryX86* m_registerFactoryX86;
		ZydisDecodedInstruction* m_curInstr = nullptr;
	public:
		DecoderX86(RegisterFactoryX86* registerFactoryX86, InstructionPool* instrPool, WarningContainer* warningContainer)
			: m_registerFactoryX86(registerFactoryX86), AbstractDecoder(instrPool, warningContainer)
		{
			ZydisDecoderInit(&m_decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
		}
	private:
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

		struct VectorOperationGeneratorInfo {
			int size = 0x0;
			int maxSize = 0x0;
			InstructionId instrId1 = InstructionId::NONE;
			InstructionId instrId2 = InstructionId::NONE;
			bool isNegate = false;
			bool isOperationWithSingleOperand = false;
			int shuffOp1[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
			int shuffOp2[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		};

		void tryDecode(void* addr, Offset offset) override;

		void translateCurInstruction();

		Varnode* getJumpOffsetByOperand(const ZydisDecodedOperand& operand);

		uint64_t getJumpOffset(int jmpOffset) const;

		void GenerateVectorOperation(const VectorOperationGeneratorInfo& info);

		Varnode* addGenericOperation(InstructionId instrId, Varnode* varnodeInput0, Varnode* varnodeInput1, Varnode* memLocVarnode = nullptr, bool isFictitious = false, int size = 0x0, int offset = 0x0);

		Instruction* addMicroInstruction(InstructionId id, Varnode* input0, Varnode* input1 = nullptr, Varnode* output = nullptr, bool zext = true);

		void setDestinationMemOperand(const ZydisDecodedOperand& operand, int size, int offset, Varnode* varnode, Varnode* memLocVarnode = nullptr);

		Varnode* requestOperandValue(const ZydisDecodedOperand& operand, int size, Varnode** memLocVarnode = nullptr, bool isMemLocLoaded = true, int memLocExprSize = 0x8);

		Varnode* requestOperandValue(const ZydisDecodedOperand& operand, int size, int offset, Varnode** memLocVarnode = nullptr, bool isMemLocLoaded = true, int memLocExprSize = 0x8);

		Varnode* GetFlagCondition(FlagCond flagCond);

		int getFirstExplicitOperandsCount() const;

		RegisterVarnode* CreateVarnode(ZydisRegister regId, int size, int offset = 0x0) const;

		RegisterVarnode* CreateVarnode(ZydisCPUFlag flag) const;
	};
};