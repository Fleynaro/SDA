#include "DecPCodeDecoderX86.h"
#include <utilities/Helper.h>

using namespace CE::Decompiler;
using namespace CE::Decompiler::PCode;

void CE::Decompiler::PCode::DecoderX86::tryDecode(void* addr, int offset) {
	auto size = m_maxSize > 0 ? (m_maxSize - offset) : 0x1000;
	ZydisDecodedInstruction curInstruction;
	if (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&m_decoder, addr, size,
		&curInstruction))) {
		m_curInstr = &curInstruction;
		m_curOrigInstr = m_instrPool->createOrigInstruction(offset, curInstruction.length);
		translateCurInstruction();
	}
}

void CE::Decompiler::PCode::DecoderX86::translateCurInstruction() {
	auto mnemonic = m_curInstr->mnemonic;
	auto size = m_curInstr->operands[0].size / 0x8;
	auto operandsCount = getFirstExplicitOperandsCount();

	switch (mnemonic)
	{
	case ZYDIS_MNEMONIC_CBW:
	case ZYDIS_MNEMONIC_CWDE:
	case ZYDIS_MNEMONIC_CDQE:
	{
		int srcRegSize = 0;
		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_CBW:
			srcRegSize = 1;
			break;
		case ZYDIS_MNEMONIC_CWDE:
			srcRegSize = 2;
			break;
		case ZYDIS_MNEMONIC_CDQE:
			srcRegSize = 4;
			break;
		}
		addMicroInstruction(InstructionId::INT_SEXT, CreateVarnode(ZYDIS_REGISTER_RAX, srcRegSize), nullptr, CreateVarnode(ZYDIS_REGISTER_RAX, srcRegSize * 2));
		break;
	}

	case ZYDIS_MNEMONIC_CWD:
	case ZYDIS_MNEMONIC_CDQ:
	case ZYDIS_MNEMONIC_CQO:
	{
		int srcRegSize = 0;
		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_CWD:
			srcRegSize = 2;
			break;
		case ZYDIS_MNEMONIC_CDQ:
			srcRegSize = 4;
			break;
		case ZYDIS_MNEMONIC_CQO:
			srcRegSize = 8;
			break;
		}

		auto varnode = m_instrPool->createSymbolVarnode(srcRegSize * 2);
		addMicroInstruction(InstructionId::INT_SEXT, CreateVarnode(ZYDIS_REGISTER_RAX, srcRegSize), nullptr, varnode);
		addMicroInstruction(InstructionId::SUBPIECE, varnode, m_instrPool->createConstantVarnode(srcRegSize, 0x4), CreateVarnode(ZYDIS_REGISTER_RDX, srcRegSize));
		break;
	}

	case ZYDIS_MNEMONIC_CVTDQ2PD:
	case ZYDIS_MNEMONIC_CVTDQ2PS:
	case ZYDIS_MNEMONIC_CVTPD2DQ:
	case ZYDIS_MNEMONIC_CVTPD2PI:
	case ZYDIS_MNEMONIC_CVTPD2PS:
	case ZYDIS_MNEMONIC_CVTPI2PD:
	case ZYDIS_MNEMONIC_CVTPI2PS:
	case ZYDIS_MNEMONIC_CVTPS2DQ:
	case ZYDIS_MNEMONIC_CVTPS2PD:
	case ZYDIS_MNEMONIC_CVTPS2PI:
	case ZYDIS_MNEMONIC_CVTSD2SI:
	case ZYDIS_MNEMONIC_CVTSD2SS:
	case ZYDIS_MNEMONIC_CVTSI2SD:
	case ZYDIS_MNEMONIC_CVTSI2SS:
	case ZYDIS_MNEMONIC_CVTSS2SD:
	case ZYDIS_MNEMONIC_CVTSS2SI:
	case ZYDIS_MNEMONIC_CVTTPD2DQ:
	case ZYDIS_MNEMONIC_CVTTPD2PI:
	case ZYDIS_MNEMONIC_CVTTPS2DQ:
	case ZYDIS_MNEMONIC_CVTTPS2PI:
	case ZYDIS_MNEMONIC_CVTTSD2SI:
	case ZYDIS_MNEMONIC_CVTTSS2SI:
	{
		InstructionId instrId;
		int dstSize;
		int srcSize;
		int maxSize = size;
		bool float2intForNonVector = false;

		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_CVTDQ2PD:
			instrId = InstructionId::INT2FLOAT;
			dstSize = 0x8;
			srcSize = 0x4;
			break;
		case ZYDIS_MNEMONIC_CVTDQ2PS:
			instrId = InstructionId::INT2FLOAT;
			dstSize = 0x4;
			srcSize = 0x4;
			break;
		case ZYDIS_MNEMONIC_CVTPD2DQ:
		case ZYDIS_MNEMONIC_CVTTPD2DQ:
			instrId = InstructionId::FLOAT2INT;
			dstSize = 0x4;
			srcSize = 0x8;
			break;
		case ZYDIS_MNEMONIC_CVTPD2PI:
			instrId = InstructionId::FLOAT2INT;
			dstSize = 0x4;
			srcSize = 0x8;
			break;
		case ZYDIS_MNEMONIC_CVTPD2PS:
			instrId = InstructionId::FLOAT2FLOAT;
			dstSize = 0x4;
			srcSize = 0x8;
			break;
		case ZYDIS_MNEMONIC_CVTPI2PD:
			instrId = InstructionId::INT2FLOAT;
			dstSize = 0x8;
			srcSize = 0x4;
			break;
		case ZYDIS_MNEMONIC_CVTPI2PS:
			instrId = InstructionId::INT2FLOAT;
			dstSize = 0x4;
			srcSize = 0x0;
			break;
		case ZYDIS_MNEMONIC_CVTPS2DQ:
		case ZYDIS_MNEMONIC_CVTTPS2DQ:
			instrId = InstructionId::FLOAT2INT;
			dstSize = 0x4;
			srcSize = 0x4;
			break;
		case ZYDIS_MNEMONIC_CVTPS2PD:
			instrId = InstructionId::FLOAT2FLOAT;
			dstSize = 0x8;
			srcSize = 0x4;
			break;
		case ZYDIS_MNEMONIC_CVTPS2PI:
			instrId = InstructionId::ROUND;
			dstSize = 0x4;
			srcSize = 0x4;
			break;
		case ZYDIS_MNEMONIC_CVTSD2SI:
			instrId = InstructionId::ROUND;
			dstSize = size;
			srcSize = 0x8;
			float2intForNonVector = true;
			break;
		case ZYDIS_MNEMONIC_CVTSD2SS:
			instrId = InstructionId::INT2FLOAT;
			maxSize = 0x4;
			dstSize = 0x4;
			srcSize = 0x8;
			break;
		case ZYDIS_MNEMONIC_CVTSI2SS:
		case ZYDIS_MNEMONIC_CVTSI2SD:
			instrId = InstructionId::INT2FLOAT;
			maxSize = 0x4;
			dstSize = 0x4;
			srcSize = size;
			break;
		case ZYDIS_MNEMONIC_CVTSS2SD:
			instrId = InstructionId::FLOAT2FLOAT;
			maxSize = 0x8;
			dstSize = 0x8;
			srcSize = 0x4;
			break;
		case ZYDIS_MNEMONIC_CVTSS2SI:
			instrId = InstructionId::ROUND;
			dstSize = size;
			srcSize = 0x4;
			float2intForNonVector = true;
			break;
		case ZYDIS_MNEMONIC_CVTTPD2PI:
			instrId = InstructionId::FLOAT2INT;
			dstSize = 0x4;
			srcSize = 0x8;
			break;
		case ZYDIS_MNEMONIC_CVTTPS2PI:
			instrId = InstructionId::FLOAT2INT;
			dstSize = 0x4;
			srcSize = 0x4;
			break;
		case ZYDIS_MNEMONIC_CVTTSD2SI:
		case ZYDIS_MNEMONIC_CVTTSS2SI:
			instrId = InstructionId::FLOAT2INT;
			dstSize = size;
			srcSize = mnemonic == ZYDIS_MNEMONIC_CVTTSS2SI ? 0x8 : 0x4;
			break;
		}

		int offset = 0;
		int offset2 = 0;
		auto& dstOperand = m_curInstr->operands[0];
		auto& srcOperand = m_curInstr->operands[1];
		while (offset < maxSize) {
			auto srcOpVarnode = requestOperandValue(srcOperand, srcSize, offset);
			auto varnodeRegOutput = CreateVarnode(dstOperand.reg.value, dstSize, offset);
			if (float2intForNonVector && !varnodeRegOutput->m_register.isVector()) {
				//all float values store in vector registers then need cast when moving to non-vector register
				auto varnodeOutput = m_instrPool->createSymbolVarnode(size);
				addMicroInstruction(instrId, srcOpVarnode, nullptr, varnodeOutput);
				addMicroInstruction(InstructionId::FLOAT2INT, varnodeOutput, nullptr, varnodeRegOutput);
			}
			else {
				addMicroInstruction(instrId, srcOpVarnode, nullptr, varnodeRegOutput);
			}
			offset += std::max(dstSize, srcSize);
			offset2 += srcSize;
		}

		while (offset2 < maxSize) {
			auto varnodeRegOutput = CreateVarnode(dstOperand.reg.value, dstSize, offset);
			addMicroInstruction(InstructionId::COPY, m_instrPool->createConstantVarnode(0x0, dstSize), nullptr, varnodeRegOutput);
			offset2 += srcSize;
		}
		break;
	}

	case ZYDIS_MNEMONIC_COMISD:
	case ZYDIS_MNEMONIC_COMISS:
	case ZYDIS_MNEMONIC_UCOMISD:
	case ZYDIS_MNEMONIC_UCOMISS:
	{
		auto op1Varnode = requestOperandValue(m_curInstr->operands[0], size);
		auto op2Varnode = requestOperandValue(m_curInstr->operands[1], size);

		auto varnodeNan1 = m_instrPool->createSymbolVarnode(1);
		addMicroInstruction(InstructionId::FLOAT_NAN, op1Varnode, nullptr, varnodeNan1);
		auto varnodeNan2 = m_instrPool->createSymbolVarnode(1);
		addMicroInstruction(InstructionId::FLOAT_NAN, op2Varnode, nullptr, varnodeNan2);
		auto flagPF = CreateVarnode(ZYDIS_CPUFLAG_PF);
		addMicroInstruction(InstructionId::BOOL_OR, varnodeNan1, varnodeNan2, flagPF);
		auto varnodeEq = m_instrPool->createSymbolVarnode(1);
		addMicroInstruction(InstructionId::FLOAT_EQUAL, op1Varnode, op2Varnode, varnodeEq);
		addMicroInstruction(InstructionId::BOOL_OR, flagPF, varnodeEq, CreateVarnode(ZYDIS_CPUFLAG_ZF));
		auto varnodeFl = m_instrPool->createSymbolVarnode(1);
		addMicroInstruction(InstructionId::FLOAT_LESS, op1Varnode, op2Varnode, varnodeFl);
		addMicroInstruction(InstructionId::BOOL_OR, flagPF, varnodeFl, CreateVarnode(ZYDIS_CPUFLAG_CF));
		auto zeroVanrnode = m_instrPool->createConstantVarnode(0x0, 1);
		addMicroInstruction(InstructionId::COPY, zeroVanrnode, nullptr, CreateVarnode(ZYDIS_CPUFLAG_OF));
		addMicroInstruction(InstructionId::COPY, zeroVanrnode, nullptr, CreateVarnode(ZYDIS_CPUFLAG_AF));
		addMicroInstruction(InstructionId::COPY, zeroVanrnode, nullptr, CreateVarnode(ZYDIS_CPUFLAG_SF));
		break;
	}

	case ZYDIS_MNEMONIC_MOVD:
	case ZYDIS_MNEMONIC_MOVSS:
	{
		auto& dstOperand = m_curInstr->operands[0];
		auto& srcOperand = m_curInstr->operands[1];
		auto srcOpVarnode = requestOperandValue(srcOperand, 0x4, 0x0);
		addGenericOperation(InstructionId::COPY, srcOpVarnode, nullptr, nullptr, false, 0x4, 0x0);
		if (mnemonic == ZYDIS_MNEMONIC_MOVD && dstOperand.type == ZYDIS_OPERAND_TYPE_REGISTER || mnemonic == ZYDIS_MNEMONIC_MOVSS && srcOperand.type == ZYDIS_OPERAND_TYPE_MEMORY) {
			auto zero = m_instrPool->createConstantVarnode(0x0, 0x4);
			addGenericOperation(InstructionId::COPY, zero, nullptr, nullptr, false, 0x4, 4);
			if (size >= 0x10) {
				addGenericOperation(InstructionId::COPY, zero, nullptr, nullptr, false, 0x4, 8);
				addGenericOperation(InstructionId::COPY, zero, nullptr, nullptr, false, 0x4, 12);
			}
		}
		break;
	}

	case ZYDIS_MNEMONIC_MOVQ:
	case ZYDIS_MNEMONIC_MOVSD:
	{
		auto& dstOperand = m_curInstr->operands[0];
		auto& srcOperand = m_curInstr->operands[1];
		auto srcOpVarnode = requestOperandValue(srcOperand, 0x8, 0x0);
		addGenericOperation(InstructionId::COPY, srcOpVarnode, nullptr, nullptr, false, 0x8, 0x0);
		if (mnemonic == ZYDIS_MNEMONIC_MOVQ && dstOperand.type != ZYDIS_OPERAND_TYPE_MEMORY) {
			if (size >= 0x10) {
				addGenericOperation(InstructionId::COPY, m_instrPool->createConstantVarnode(0x0, 0x8), nullptr, nullptr, false, 0x8, 0x8);
			}
		}
		break;
	}

	//unpckh
	case ZYDIS_MNEMONIC_UNPCKHPD:
	case ZYDIS_MNEMONIC_UNPCKHPS:
		//unpckl
	case ZYDIS_MNEMONIC_UNPCKLPD:
	case ZYDIS_MNEMONIC_UNPCKLPS:
	{
		auto& dstOperand = m_curInstr->operands[0];
		auto& srcOperand = m_curInstr->operands[1];
		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_UNPCKHPD: {
			auto dstOpVarnode = requestOperandValue(dstOperand, 0x8, 0x8);
			auto srcOpVarnode = requestOperandValue(srcOperand, 0x8, 0x8);
			addGenericOperation(InstructionId::COPY, dstOpVarnode, nullptr, nullptr, false, 0x8, 0x0);
			addGenericOperation(InstructionId::COPY, srcOpVarnode, nullptr, nullptr, false, 0x8, 0x8);
			break;
		}
		case ZYDIS_MNEMONIC_UNPCKHPS: {
			auto dstOpVarnode1 = requestOperandValue(dstOperand, 0x4, 8);
			auto srcOpVarnode1 = requestOperandValue(srcOperand, 0x4, 8);
			auto dstOpVarnode2 = requestOperandValue(dstOperand, 0x4, 12);
			auto srcOpVarnode2 = requestOperandValue(srcOperand, 0x4, 12);
			addGenericOperation(InstructionId::COPY, dstOpVarnode1, nullptr, nullptr, false, 0x4, 0);
			addGenericOperation(InstructionId::COPY, srcOpVarnode1, nullptr, nullptr, false, 0x4, 4);
			addGenericOperation(InstructionId::COPY, dstOpVarnode2, nullptr, nullptr, false, 0x4, 8);
			addGenericOperation(InstructionId::COPY, srcOpVarnode2, nullptr, nullptr, false, 0x4, 12);
			break;
		}
		case ZYDIS_MNEMONIC_UNPCKLPD: {
			auto srcOpVarnode = requestOperandValue(srcOperand, 0x8, 0x0);
			addGenericOperation(InstructionId::COPY, srcOpVarnode, nullptr, nullptr, false, 0x8, 0x8);
			break;
		}
		case ZYDIS_MNEMONIC_UNPCKLPS: {
			auto dstOpVarnode = requestOperandValue(dstOperand, 0x4, 4);
			auto srcOpVarnode1 = requestOperandValue(srcOperand, 0x4, 4);
			auto srcOpVarnode2 = requestOperandValue(srcOperand, 0x4, 0);
			addGenericOperation(InstructionId::COPY, dstOpVarnode, nullptr, nullptr, false, 0x4, 8);
			addGenericOperation(InstructionId::COPY, srcOpVarnode1, nullptr, nullptr, false, 0x4, 12);
			addGenericOperation(InstructionId::COPY, srcOpVarnode2, nullptr, nullptr, false, 0x4, 4);
			break;
		}
		}
		break;
	}

	//mov
	case ZYDIS_MNEMONIC_MOVAPD:
	case ZYDIS_MNEMONIC_MOVAPS:
	case ZYDIS_MNEMONIC_MOVUPD:
	case ZYDIS_MNEMONIC_MOVUPS:
		//blend
	case ZYDIS_MNEMONIC_BLENDPD:
	case ZYDIS_MNEMONIC_BLENDPS:
		//shuff
	case ZYDIS_MNEMONIC_SHUFPD:
	case ZYDIS_MNEMONIC_SHUFPS:
		//addsub
	case ZYDIS_MNEMONIC_ADDSUBPD:
	case ZYDIS_MNEMONIC_ADDSUBPS:
		//hadd
	case ZYDIS_MNEMONIC_HADDPD:
	case ZYDIS_MNEMONIC_HADDPS:
		//hsub
	case ZYDIS_MNEMONIC_HSUBPD:
	case ZYDIS_MNEMONIC_HSUBPS:
		//add
	case ZYDIS_MNEMONIC_ADDPD:
	case ZYDIS_MNEMONIC_ADDPS:
	case ZYDIS_MNEMONIC_ADDSD:
	case ZYDIS_MNEMONIC_ADDSS:
		//sub
	case ZYDIS_MNEMONIC_SUBPD:
	case ZYDIS_MNEMONIC_SUBPS:
	case ZYDIS_MNEMONIC_SUBSD:
	case ZYDIS_MNEMONIC_SUBSS:
		//mul
	case ZYDIS_MNEMONIC_MULPD:
	case ZYDIS_MNEMONIC_MULPS:
	case ZYDIS_MNEMONIC_MULSD:
	case ZYDIS_MNEMONIC_MULSS:
		//div
	case ZYDIS_MNEMONIC_DIVPD:
	case ZYDIS_MNEMONIC_DIVPS:
	case ZYDIS_MNEMONIC_DIVSD:
	case ZYDIS_MNEMONIC_DIVSS:
		//and
	case ZYDIS_MNEMONIC_ANDPD:
	case ZYDIS_MNEMONIC_ANDPS:
		//andnot
	case ZYDIS_MNEMONIC_ANDNPD:
	case ZYDIS_MNEMONIC_ANDNPS:
		//or
	case ZYDIS_MNEMONIC_ORPD:
	case ZYDIS_MNEMONIC_ORPS:
		//xor
	case ZYDIS_MNEMONIC_XORPD:
	case ZYDIS_MNEMONIC_XORPS:
		//sqrt
	case ZYDIS_MNEMONIC_SQRTPD:
	case ZYDIS_MNEMONIC_SQRTPS:
	case ZYDIS_MNEMONIC_SQRTSD:
	case ZYDIS_MNEMONIC_SQRTSS:
	{
		VectorOperationGeneratorInfo info;
		info.maxSize = size;

		enum class OperationSize {
			PD,
			PS,
			SD,
			SS
		};
		OperationSize operationSize;

		if (ZYDIS_MNEMONIC_MOVUPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_MOVUPS) {
			info.instrId1 = InstructionId::COPY;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_MOVUPD);
		}
		else if (ZYDIS_MNEMONIC_MOVAPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_MOVAPS) {
			info.instrId1 = InstructionId::COPY;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_MOVAPD);
		}
		else if (ZYDIS_MNEMONIC_BLENDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_BLENDPS) {
			info.instrId1 = InstructionId::COPY;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_BLENDPD);
			auto& infoOperand = m_curInstr->operands[2];
			for (int i = 0; i < 4; i++) {
				info.shuffOp2[i] = ((infoOperand.imm.value.u >> i) & 0b1) ? info.shuffOp2[i] : -1;
			}
		}
		else if (ZYDIS_MNEMONIC_SHUFPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_SHUFPS) {
			info.instrId1 = InstructionId::COPY;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_SHUFPD);
			info.isOperationWithSingleOperand = true;
			auto& infoOperand = m_curInstr->operands[2];
			if (operationSize == OperationSize::PS) {
				for (int i = 0; i < 4; i++) {
					info.shuffOp1[i] = info.shuffOp2[i] = ((infoOperand.imm.value.u >> (i * 2)) & 0b11);
				}
			}
			else {
				for (int i = 0; i < 2; i++) {
					info.shuffOp1[i] = info.shuffOp2[i] = ((infoOperand.imm.value.u >> (i * 1)) & 0b1);
				}
			}
		}
		else if (ZYDIS_MNEMONIC_ADDSUBPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_ADDSUBPS) {
			info.instrId1 = InstructionId::FLOAT_SUB;
			info.instrId2 = InstructionId::FLOAT_ADD;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_ADDSUBPD);
		}
		else if (ZYDIS_MNEMONIC_HADDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_HADDPS || ZYDIS_MNEMONIC_HSUBPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_HSUBPS) {
			if (ZYDIS_MNEMONIC_HADDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_HADDPS) {
				info.instrId1 = InstructionId::INT_ADD;
				operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_HADDPD);
			}
			else {
				info.instrId1 = InstructionId::INT_SUB;
				operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_HSUBPD);
			}

			if (operationSize == OperationSize::PS) {
				info.shuffOp1[0] = info.shuffOp1[2] = 0;
				info.shuffOp1[1] = info.shuffOp1[3] = 2;
				info.shuffOp2[0] = info.shuffOp2[2] = 1;
				info.shuffOp2[1] = info.shuffOp2[3] = 3;
			}
			else {
				info.shuffOp1[0] = info.shuffOp1[1] = 0;
				info.shuffOp2[0] = info.shuffOp2[1] = 1;
			}
		}
		else if (ZYDIS_MNEMONIC_HSUBPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_HSUBPS) {
			info.instrId1 = InstructionId::INT_ADD;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_HSUBPD);
		}
		else if (ZYDIS_MNEMONIC_ADDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_ADDSS) {
			info.instrId1 = InstructionId::FLOAT_ADD;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_ADDPD);
		}
		else if (ZYDIS_MNEMONIC_SUBPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_SUBSS) {
			info.instrId1 = InstructionId::FLOAT_SUB;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_SUBPD);
		}
		else if (ZYDIS_MNEMONIC_MULPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_MULSS) {
			info.instrId1 = InstructionId::FLOAT_MULT;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_MULPD);
		}
		else if (ZYDIS_MNEMONIC_DIVPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_DIVSS) {
			info.instrId1 = InstructionId::FLOAT_DIV;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_DIVPD);
		}
		else if (ZYDIS_MNEMONIC_ANDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_ANDPS) {
			info.instrId1 = InstructionId::INT_AND;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_ANDPD);
		}
		else if (ZYDIS_MNEMONIC_ANDNPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_ANDNPS) {
			info.instrId1 = InstructionId::INT_AND;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_ANDNPD);
			info.isNegate = true;
		}
		else if (ZYDIS_MNEMONIC_ORPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_ORPS) {
			info.instrId1 = InstructionId::INT_OR;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_ORPD);
		}
		else if (ZYDIS_MNEMONIC_XORPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_XORPS) {
			info.instrId1 = InstructionId::INT_XOR;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_XORPD);
		}
		else if (ZYDIS_MNEMONIC_SQRTPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_SQRTSS) {
			info.instrId1 = InstructionId::FLOAT_SQRT;
			operationSize = OperationSize(mnemonic - ZYDIS_MNEMONIC_SQRTPD);
		}

		switch (operationSize)
		{
		case OperationSize::PS:
			info.size = 0x4;
			break;
		case OperationSize::PD:
			info.size = 0x8;
			break;
		case OperationSize::SS:
			info.maxSize = info.size = 0x4;
			break;
		case OperationSize::SD:
			info.maxSize = info.size = 0x8;
			break;
		}
		GenerateVectorOperation(info);
		break;
	}

	//add
	case ZYDIS_MNEMONIC_PADDB:
	case ZYDIS_MNEMONIC_PADDW:
	case ZYDIS_MNEMONIC_PADDD:
	case ZYDIS_MNEMONIC_PADDQ:
		//sub
	case ZYDIS_MNEMONIC_PSUBB:
	case ZYDIS_MNEMONIC_PSUBW:
	case ZYDIS_MNEMONIC_PSUBD:
	case ZYDIS_MNEMONIC_PSUBQ:
		//shuf
	case ZYDIS_MNEMONIC_PSHUFD:
	case ZYDIS_MNEMONIC_PSHUFHW:
	case ZYDIS_MNEMONIC_PSHUFLW:
	case ZYDIS_MNEMONIC_PSHUFW:
	{
		VectorOperationGeneratorInfo info;
		info.maxSize = size;

		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_PADDB:
		case ZYDIS_MNEMONIC_PADDW:
		case ZYDIS_MNEMONIC_PADDD:
		case ZYDIS_MNEMONIC_PADDQ:
		{
			info.instrId1 = InstructionId::INT_ADD;
			switch (mnemonic)
			{
			case ZYDIS_MNEMONIC_PADDB:
				info.size = 0x1;
				break;
			case ZYDIS_MNEMONIC_PADDW:
				info.size = 0x2;
				break;
			case ZYDIS_MNEMONIC_PADDD:
				info.size = 0x4;
				break;
			case ZYDIS_MNEMONIC_PADDQ:
				info.size = 0x8;
				break;
			}
			break;
		}

		case ZYDIS_MNEMONIC_PSUBB:
		case ZYDIS_MNEMONIC_PSUBW:
		case ZYDIS_MNEMONIC_PSUBD:
		case ZYDIS_MNEMONIC_PSUBQ:
		{
			info.instrId1 = InstructionId::INT_SUB;
			switch (mnemonic)
			{
			case ZYDIS_MNEMONIC_PSUBB:
				info.size = 0x1;
				break;
			case ZYDIS_MNEMONIC_PSUBW:
				info.size = 0x2;
				break;
			case ZYDIS_MNEMONIC_PSUBD:
				info.size = 0x4;
				break;
			case ZYDIS_MNEMONIC_PSUBQ:
				info.size = 0x8;
				break;
			}
			break;
		}

		case ZYDIS_MNEMONIC_PSHUFD:
		case ZYDIS_MNEMONIC_PSHUFHW:
		case ZYDIS_MNEMONIC_PSHUFLW:
		case ZYDIS_MNEMONIC_PSHUFW:
		{
			info.instrId1 = InstructionId::COPY;
			auto& infoOperand = m_curInstr->operands[2];
			int startIdx = 0;
			switch (mnemonic)
			{
			case ZYDIS_MNEMONIC_PSHUFD:
				info.size = 0x4;
				break;
			case ZYDIS_MNEMONIC_PSHUFHW:
			case ZYDIS_MNEMONIC_PSHUFLW:
			case ZYDIS_MNEMONIC_PSHUFW:
				info.size = 0x2;
				if (mnemonic == ZYDIS_MNEMONIC_PSHUFHW)
					startIdx = 4;
				break;
			}

			for (int i = 0; i < 4; i++) {
				info.shuffOp1[startIdx + i] = info.shuffOp2[startIdx + i] = startIdx + ((infoOperand.imm.value.u >> (i * 2)) & 0b11);
			}
			break;
		}
		}

		GenerateVectorOperation(info);
		break;
	}

	case ZYDIS_MNEMONIC_XCHG:
	{
		Varnode* varnodeInput0 = requestOperandValue(m_curInstr->operands[0], size);
		Varnode* varnodeInput1 = requestOperandValue(m_curInstr->operands[1], size);
		auto varnodeTemp = m_instrPool->createSymbolVarnode(size);
		addMicroInstruction(InstructionId::COPY, varnodeInput0, nullptr, varnodeTemp);
		addMicroInstruction(InstructionId::COPY, varnodeInput1, nullptr, varnodeInput0);
		addMicroInstruction(InstructionId::COPY, varnodeTemp, nullptr, varnodeInput1);
		break;
	}

	case ZYDIS_MNEMONIC_MOV:
	case ZYDIS_MNEMONIC_MOVZX:
	case ZYDIS_MNEMONIC_MOVSX:
	case ZYDIS_MNEMONIC_MOVSXD:
	case ZYDIS_MNEMONIC_LEA:
	{
		auto& operand = m_curInstr->operands[1];
		Varnode* varnode;
		if (mnemonic == ZYDIS_MNEMONIC_LEA) {
			varnode = requestOperandValue(operand, operand.size / 0x8, nullptr, false, size);
		}
		else {
			varnode = requestOperandValue(operand, operand.size / 0x8, nullptr);
		}

		auto instrId = InstructionId::COPY;
		switch (mnemonic) {
		case ZYDIS_MNEMONIC_MOVSX:
			instrId = InstructionId::INT_SEXT;
			break;
		case ZYDIS_MNEMONIC_MOVZX:
			instrId = InstructionId::INT_ZEXT;
			break;
		}
		addGenericOperation(instrId, varnode, nullptr);
		break;
	}

	case ZYDIS_MNEMONIC_ADD:
	case ZYDIS_MNEMONIC_INC:
	case ZYDIS_MNEMONIC_DEC:
	case ZYDIS_MNEMONIC_SUB:
	case ZYDIS_MNEMONIC_CMP:
	case ZYDIS_MNEMONIC_NEG:
	case ZYDIS_MNEMONIC_MUL:
	case ZYDIS_MNEMONIC_IMUL:
	case ZYDIS_MNEMONIC_DIV:
	case ZYDIS_MNEMONIC_IDIV:
	case ZYDIS_MNEMONIC_AND:
	case ZYDIS_MNEMONIC_TEST:
	case ZYDIS_MNEMONIC_OR:
	case ZYDIS_MNEMONIC_XOR:
	case ZYDIS_MNEMONIC_SHL:
	case ZYDIS_MNEMONIC_SHR:
	case ZYDIS_MNEMONIC_SAR:
	case ZYDIS_MNEMONIC_BT:
	case ZYDIS_MNEMONIC_BTR:
	{
		Varnode* memLocVarnode = nullptr;
		Varnode* varnodeInput0 = requestOperandValue(m_curInstr->operands[0], size, &memLocVarnode);
		Varnode* varnodeInput1 = nullptr;
		Varnode* varnodeInput2 = nullptr;
		Varnode* varnodeOutput = nullptr;
		if (operandsCount >= 2)
			varnodeInput1 = requestOperandValue(m_curInstr->operands[1], size);
		if (operandsCount >= 3)
			varnodeInput2 = requestOperandValue(m_curInstr->operands[2], size);

		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_ADD:
		case ZYDIS_MNEMONIC_INC:
		case ZYDIS_MNEMONIC_DEC:
		{
			auto flagOfInstrId = InstructionId::INT_SCARRY;
			if (mnemonic == ZYDIS_MNEMONIC_INC || mnemonic == ZYDIS_MNEMONIC_DEC) {
				varnodeInput1 = m_instrPool->createConstantVarnode(0x1, size);
				if (mnemonic == ZYDIS_MNEMONIC_DEC) {
					flagOfInstrId = InstructionId::INT_SBORROW;
				}
			}
			else {
				addMicroInstruction(InstructionId::INT_CARRY, varnodeInput0, varnodeInput1, CreateVarnode(ZYDIS_CPUFLAG_CF));
			}
			addMicroInstruction(flagOfInstrId, varnodeInput0, varnodeInput1, CreateVarnode(ZYDIS_CPUFLAG_OF));
			varnodeOutput = addGenericOperation(mnemonic == ZYDIS_MNEMONIC_DEC ? InstructionId::INT_SUB : InstructionId::INT_ADD, varnodeInput0, varnodeInput1, memLocVarnode);
			break;
		}

		case ZYDIS_MNEMONIC_SUB:
		case ZYDIS_MNEMONIC_CMP:
		{
			addMicroInstruction(InstructionId::INT_LESS, varnodeInput0, varnodeInput1, CreateVarnode(ZYDIS_CPUFLAG_CF));
			addMicroInstruction(InstructionId::INT_SBORROW, varnodeInput0, varnodeInput1, CreateVarnode(ZYDIS_CPUFLAG_OF));
			varnodeOutput = addGenericOperation(InstructionId::INT_SUB, varnodeInput0, varnodeInput1, memLocVarnode, mnemonic == ZYDIS_MNEMONIC_CMP);
			break;
		}

		case ZYDIS_MNEMONIC_NEG:
		{
			auto varnodeZero = m_instrPool->createConstantVarnode(0x0, size);
			addMicroInstruction(InstructionId::INT_NOTEQUAL, varnodeInput0, varnodeZero, CreateVarnode(ZYDIS_CPUFLAG_CF));
			addMicroInstruction(InstructionId::INT_SBORROW, varnodeZero, varnodeInput0, CreateVarnode(ZYDIS_CPUFLAG_OF));
			varnodeOutput = addGenericOperation(InstructionId::INT_2COMP, varnodeInput0, nullptr, memLocVarnode);
			break;
		}

		case ZYDIS_MNEMONIC_MUL:
		case ZYDIS_MNEMONIC_IMUL:
		{
			Varnode* varnodeDst = nullptr;
			Varnode* varnodeMul1;
			Varnode* varnodeMul2;
			Varnode* varnodeCF = CreateVarnode(ZYDIS_CPUFLAG_CF);

			if (operandsCount == 1) {
				varnodeDst = varnodeMul1 = CreateVarnode(ZYDIS_REGISTER_RAX, size);
				varnodeMul2 = varnodeInput0;
			}
			else if (operandsCount == 2) {
				varnodeDst = varnodeMul1 = varnodeInput0;
				varnodeMul2 = varnodeInput1;
			}
			else {
				varnodeDst = varnodeInput0;
				varnodeMul1 = varnodeInput1;
				varnodeMul2 = varnodeInput2;
			}

			auto instrExt = InstructionId::INT_ZEXT;
			if (mnemonic == ZYDIS_MNEMONIC_IMUL)
				instrExt = InstructionId::INT_SEXT;

			auto varnodeZext1 = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(instrExt, varnodeMul1, nullptr, varnodeZext1);
			auto varnodeZext2 = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(instrExt, varnodeMul2, nullptr, varnodeZext2);
			auto varnodeMult = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(InstructionId::INT_MULT, varnodeZext1, varnodeZext2, varnodeMult);

			Varnode* varnodeSubpiece;
			if (operandsCount == 1) {
				varnodeSubpiece = CreateVarnode(ZYDIS_REGISTER_RDX, size);
			}
			else {
				varnodeSubpiece = m_instrPool->createSymbolVarnode(size * 2);
			}
			if (mnemonic == ZYDIS_MNEMONIC_IMUL) {
				addMicroInstruction(InstructionId::INT_MULT, varnodeMul1, varnodeMul2, varnodeDst);
				addMicroInstruction(InstructionId::SUBPIECE, varnodeMult, m_instrPool->createConstantVarnode(size, 0x4), varnodeSubpiece);
				auto varnodeNe1 = m_instrPool->createSymbolVarnode(0x1);
				addMicroInstruction(InstructionId::INT_NOTEQUAL, varnodeSubpiece, m_instrPool->createConstantVarnode(0x0, size), varnodeNe1);
				auto varnode2Cmp = m_instrPool->createSymbolVarnode(size);
				addMicroInstruction(InstructionId::INT_2COMP, m_instrPool->createConstantVarnode(0x1, size), nullptr, varnode2Cmp);
				auto varnodeNe2 = m_instrPool->createSymbolVarnode(0x1);
				addMicroInstruction(InstructionId::INT_NOTEQUAL, varnodeSubpiece, varnode2Cmp, varnodeNe2);
				addMicroInstruction(InstructionId::INT_AND, varnodeNe1, varnodeNe2, varnodeCF);
			}
			else {
				addMicroInstruction(InstructionId::SUBPIECE, varnodeMult, m_instrPool->createConstantVarnode(size, 0x4), varnodeSubpiece);
				addMicroInstruction(InstructionId::INT_MULT, varnodeMul1, varnodeMul2, varnodeDst); //not like ghidra
				addMicroInstruction(InstructionId::INT_NOTEQUAL, varnodeSubpiece, m_instrPool->createConstantVarnode(0x0, size), varnodeCF);
			}

			addMicroInstruction(InstructionId::COPY, varnodeCF, nullptr, CreateVarnode(ZYDIS_CPUFLAG_OF));
			break;
		}

		case ZYDIS_MNEMONIC_DIV:
		case ZYDIS_MNEMONIC_IDIV:
		{
			auto instrExt = InstructionId::INT_ZEXT;
			auto instrDiv = InstructionId::INT_DIV;
			auto instrRem = InstructionId::INT_REM;
			if (mnemonic == ZYDIS_MNEMONIC_IDIV) {
				instrExt = InstructionId::INT_SEXT;
				instrDiv = InstructionId::INT_SDIV;
				instrRem = InstructionId::INT_SREM;
			}

			auto varnodeRax = CreateVarnode(ZYDIS_REGISTER_RAX, size);
			auto varnodeRdx = CreateVarnode(ZYDIS_REGISTER_RDX, size);
			auto varnodeExt = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(instrExt, varnodeInput0, nullptr, varnodeExt);

			auto varnodeZext1 = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(InstructionId::INT_ZEXT, varnodeRdx, nullptr, varnodeZext1);
			auto varnodeLeft = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(InstructionId::INT_LEFT, varnodeZext1, m_instrPool->createConstantVarnode(size * 0x8, 0x4), varnodeLeft);
			auto varnodeZext2 = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(InstructionId::INT_ZEXT, varnodeRax, nullptr, varnodeZext2);
			auto varnodeOr = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(InstructionId::INT_OR, varnodeLeft, varnodeZext2, varnodeOr);

			auto varnodeDiv = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(instrDiv, varnodeOr, varnodeExt, varnodeDiv);

			addMicroInstruction(InstructionId::SUBPIECE, varnodeDiv, m_instrPool->createConstantVarnode(0x0, 0x4), varnodeRax);
			auto varnodeRem = m_instrPool->createSymbolVarnode(size * 2);
			addMicroInstruction(instrRem, varnodeOr, varnodeExt, varnodeRem);
			addMicroInstruction(InstructionId::SUBPIECE, varnodeRem, m_instrPool->createConstantVarnode(0x0, 0x4), varnodeRdx);
			break;
		}

		case ZYDIS_MNEMONIC_AND:
		case ZYDIS_MNEMONIC_TEST:
		case ZYDIS_MNEMONIC_OR:
		case ZYDIS_MNEMONIC_XOR:
		{
			auto instrId = InstructionId::NONE;
			switch (mnemonic) {
			case ZYDIS_MNEMONIC_AND:
			case ZYDIS_MNEMONIC_ANDN:
			case ZYDIS_MNEMONIC_TEST:
				instrId = InstructionId::INT_AND;
				break;
			case ZYDIS_MNEMONIC_OR:
				instrId = InstructionId::INT_OR;
				break;
			case ZYDIS_MNEMONIC_XOR:
				instrId = InstructionId::INT_XOR;
				break;
			}

			if (mnemonic == ZYDIS_MNEMONIC_ANDN) {
				varnodeInput0 = addGenericOperation(InstructionId::INT_NEGATE, varnodeInput0, nullptr);
			}
			addMicroInstruction(InstructionId::COPY, m_instrPool->createConstantVarnode(0x0, size), nullptr, CreateVarnode(ZYDIS_CPUFLAG_CF));
			addMicroInstruction(InstructionId::COPY, m_instrPool->createConstantVarnode(0x0, size), nullptr, CreateVarnode(ZYDIS_CPUFLAG_OF));
			varnodeOutput = addGenericOperation(instrId, varnodeInput0, varnodeInput1, memLocVarnode, mnemonic == ZYDIS_MNEMONIC_TEST);
			break;
		}

		case ZYDIS_MNEMONIC_SHL:
		case ZYDIS_MNEMONIC_SHR:
		case ZYDIS_MNEMONIC_SAR:
		{
			auto instrId = InstructionId::NONE;
			switch (mnemonic) {
			case ZYDIS_MNEMONIC_SHL:
				instrId = InstructionId::INT_LEFT;
				break;
			case ZYDIS_MNEMONIC_SHR:
				instrId = InstructionId::INT_RIGHT;
				break;
			case ZYDIS_MNEMONIC_SAR:
				instrId = InstructionId::INT_SRIGHT;
				break;
			}
			auto varnodeAndInput1 = m_instrPool->createSymbolVarnode(0x8);
			addMicroInstruction(InstructionId::INT_AND, varnodeInput1, m_instrPool->createConstantVarnode(63, size), varnodeAndInput1);
			addGenericOperation(instrId, varnodeInput0, varnodeAndInput1, memLocVarnode);
			//flags ...
			break;
		}

		case ZYDIS_MNEMONIC_BT:
		case ZYDIS_MNEMONIC_BTR:
		{
			auto varnodeAndInput1 = m_instrPool->createSymbolVarnode(0x8);
			addMicroInstruction(InstructionId::INT_AND, varnodeInput1, m_instrPool->createConstantVarnode(63, size), varnodeAndInput1);
			auto varnodeRight = m_instrPool->createSymbolVarnode(0x8);
			addMicroInstruction(InstructionId::INT_RIGHT, varnodeInput0, varnodeAndInput1, varnodeRight);
			auto varnodeAnd = m_instrPool->createSymbolVarnode(0x8);
			addMicroInstruction(InstructionId::INT_AND, varnodeRight, m_instrPool->createConstantVarnode(1, size), varnodeAnd);

			if (mnemonic != ZYDIS_MNEMONIC_BT) {
				auto varnodeLeft = m_instrPool->createSymbolVarnode(0x8);
				addMicroInstruction(InstructionId::INT_LEFT, m_instrPool->createConstantVarnode(1, size), varnodeAndInput1, varnodeLeft);
				auto varnodeNegate = m_instrPool->createSymbolVarnode(0x8);
				addMicroInstruction(InstructionId::INT_NEGATE, varnodeLeft, nullptr, varnodeNegate);
				addGenericOperation(InstructionId::INT_AND, varnodeInput0, varnodeNegate, memLocVarnode);
			}

			addMicroInstruction(InstructionId::INT_NOTEQUAL, varnodeAnd, m_instrPool->createConstantVarnode(0x0, size), CreateVarnode(ZYDIS_CPUFLAG_CF));
			break;
		}
		}

		if (varnodeOutput) {
			addMicroInstruction(InstructionId::INT_SLESS, varnodeOutput, m_instrPool->createConstantVarnode(0x0, size), CreateVarnode(ZYDIS_CPUFLAG_SF));
			addMicroInstruction(InstructionId::INT_EQUAL, varnodeOutput, m_instrPool->createConstantVarnode(0x0, size), CreateVarnode(ZYDIS_CPUFLAG_ZF));
		}
		break;
	}

	case ZYDIS_MNEMONIC_NOT:
	{
		Varnode* memLocVarnode = nullptr;
		auto varnodeInput0 = requestOperandValue(m_curInstr->operands[0], size, &memLocVarnode);
		addGenericOperation(InstructionId::INT_NEGATE, varnodeInput0, nullptr, memLocVarnode);
		break;
	}

	case ZYDIS_MNEMONIC_PUSH:
	{
		auto varnodeReg = requestOperandValue(m_curInstr->operands[0], size);
		auto varnodeRsp = CreateVarnode(ZYDIS_REGISTER_RSP, 0x8);
		addMicroInstruction(InstructionId::INT_SUB, varnodeRsp, m_instrPool->createConstantVarnode(size, 0x8), varnodeRsp);
		addMicroInstruction(InstructionId::STORE, varnodeRsp, varnodeReg);
		break;
	}

	case ZYDIS_MNEMONIC_POP:
	{
		auto varnodeReg = requestOperandValue(m_curInstr->operands[0], size);
		auto varnodeRsp = CreateVarnode(ZYDIS_REGISTER_RSP, 0x8);
		addMicroInstruction(InstructionId::LOAD, varnodeRsp, nullptr, varnodeReg);
		addMicroInstruction(InstructionId::INT_ADD, varnodeRsp, m_instrPool->createConstantVarnode(size, 0x8), varnodeRsp);
		break;
	}

	case ZYDIS_MNEMONIC_RET:
	{
		auto varnodeRip = CreateVarnode(ZYDIS_REGISTER_RIP, 0x8);
		auto varnodeRsp = CreateVarnode(ZYDIS_REGISTER_RSP, 0x8);
		addMicroInstruction(InstructionId::LOAD, varnodeRsp, nullptr, varnodeRip);
		addMicroInstruction(InstructionId::INT_ADD, varnodeRsp, m_instrPool->createConstantVarnode(size, 0x8), varnodeRsp);
		addMicroInstruction(InstructionId::RETURN, varnodeRip, nullptr);
		break;
	}

	case ZYDIS_MNEMONIC_JMP:
	case ZYDIS_MNEMONIC_CALL:
	{
		auto varnodeInput0 = getJumpOffsetByOperand(m_curInstr->operands[0]);
		if (mnemonic == ZYDIS_MNEMONIC_JMP) {
			addMicroInstruction(InstructionId::BRANCH, varnodeInput0, nullptr);
		}
		else {
			/*if (false) {
			auto varnodeRsp = CreateVarnode(ZYDIS_REGISTER_RSP, 0x8);
			addMicroInstruction(InstructionId::INT_SUB, varnodeRsp, m_instrPool->createConstantVarnode(0x8, 0x8), varnodeRsp);
			auto offset = getJumpOffsetToNextInstr();
			addMicroInstruction(InstructionId::STORE, varnodeRsp, m_instrPool->createConstantVarnode((uint64_t&)offset, 0x8));
			}*/
			if (auto varnodeConst = dynamic_cast<ConstantVarnode*>(varnodeInput0)) {
				varnodeConst->m_value = (int64_t)(int)(varnodeConst->m_value >> 8);
				auto varnodeRip = CreateVarnode(ZYDIS_REGISTER_RIP, 0x8);
				auto varnodeDst = m_instrPool->createSymbolVarnode(0x8);
				addMicroInstruction(InstructionId::INT_ADD, varnodeRip, varnodeConst, varnodeDst);
				varnodeInput0 = varnodeDst;
			}
			addMicroInstruction(InstructionId::CALL, varnodeInput0, nullptr);
		}
		break;
	}

	case ZYDIS_MNEMONIC_CMOVB:
	case ZYDIS_MNEMONIC_CMOVBE:
	case ZYDIS_MNEMONIC_CMOVL:
	case ZYDIS_MNEMONIC_CMOVLE:
	case ZYDIS_MNEMONIC_CMOVNB:
	case ZYDIS_MNEMONIC_CMOVNBE:
	case ZYDIS_MNEMONIC_CMOVNL:
	case ZYDIS_MNEMONIC_CMOVNLE:
	case ZYDIS_MNEMONIC_CMOVNO:
	case ZYDIS_MNEMONIC_CMOVNP:
	case ZYDIS_MNEMONIC_CMOVNS:
	case ZYDIS_MNEMONIC_CMOVNZ:
	case ZYDIS_MNEMONIC_CMOVO:
	case ZYDIS_MNEMONIC_CMOVP:
	case ZYDIS_MNEMONIC_CMOVS:
	case ZYDIS_MNEMONIC_CMOVZ:
	{
		FlagCond flagCond = FlagCond::NONE;
		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_CMOVB:
			flagCond = FlagCond::C;
			break;
		case ZYDIS_MNEMONIC_CMOVBE:
			flagCond = FlagCond::NA;
			break;
		case ZYDIS_MNEMONIC_CMOVL:
			flagCond = FlagCond::L;
			break;
		case ZYDIS_MNEMONIC_CMOVLE:
			flagCond = FlagCond::LE;
			break;
		case ZYDIS_MNEMONIC_CMOVNB:
			flagCond = FlagCond::NC;
			break;
		case ZYDIS_MNEMONIC_CMOVNBE:
			flagCond = FlagCond::A;
			break;
		case ZYDIS_MNEMONIC_CMOVNL:
			flagCond = FlagCond::NL;
			break;
		case ZYDIS_MNEMONIC_CMOVNLE:
			flagCond = FlagCond::NLE;
			break;
		case ZYDIS_MNEMONIC_CMOVNO:
			flagCond = FlagCond::O;
			break;
		case ZYDIS_MNEMONIC_CMOVNP:
			flagCond = FlagCond::NP;
			break;
		case ZYDIS_MNEMONIC_CMOVNS:
			flagCond = FlagCond::S;
			break;
		case ZYDIS_MNEMONIC_CMOVNZ:
			flagCond = FlagCond::NZ;
			break;
		case ZYDIS_MNEMONIC_CMOVO:
			flagCond = FlagCond::O;
			break;
		case ZYDIS_MNEMONIC_CMOVP:
			flagCond = FlagCond::P;
			break;
		case ZYDIS_MNEMONIC_CMOVS:
			flagCond = FlagCond::S;
			break;
		case ZYDIS_MNEMONIC_CMOVZ:
			flagCond = FlagCond::Z;
			break;
		}

		auto varnodeFlagCond = GetFlagCondition(flagCond);
		auto varnodeNeg = m_instrPool->createSymbolVarnode(1);
		addMicroInstruction(InstructionId::BOOL_NEGATE, varnodeFlagCond, nullptr, varnodeNeg);
		auto varnodeNextInstrOffset = m_instrPool->createConstantVarnode(getJumpOffset(0x0), 0x8);
		addMicroInstruction(InstructionId::CBRANCH, varnodeNextInstrOffset, varnodeNeg);
		auto operand = m_curInstr->operands[1];
		auto varnode = requestOperandValue(operand, size, nullptr, operand.actions != 0);
		addGenericOperation(InstructionId::COPY, varnode, nullptr);
		break;
	}

	case ZYDIS_MNEMONIC_SETB:
	case ZYDIS_MNEMONIC_SETBE:
	case ZYDIS_MNEMONIC_SETL:
	case ZYDIS_MNEMONIC_SETLE:
	case ZYDIS_MNEMONIC_SETNB:
	case ZYDIS_MNEMONIC_SETNBE:
	case ZYDIS_MNEMONIC_SETNL:
	case ZYDIS_MNEMONIC_SETNLE:
	case ZYDIS_MNEMONIC_SETNO:
	case ZYDIS_MNEMONIC_SETNP:
	case ZYDIS_MNEMONIC_SETNS:
	case ZYDIS_MNEMONIC_SETNZ:
	case ZYDIS_MNEMONIC_SETO:
	case ZYDIS_MNEMONIC_SETP:
	case ZYDIS_MNEMONIC_SETS:
	case ZYDIS_MNEMONIC_SETSSBSY:
	case ZYDIS_MNEMONIC_SETZ:
	{
		FlagCond flagCond = FlagCond::NONE;
		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_SETB:
			flagCond = FlagCond::C;
			break;
		case ZYDIS_MNEMONIC_SETBE:
			flagCond = FlagCond::NA;
			break;
		case ZYDIS_MNEMONIC_SETL:
			flagCond = FlagCond::L;
			break;
		case ZYDIS_MNEMONIC_SETLE:
			flagCond = FlagCond::LE;
			break;
		case ZYDIS_MNEMONIC_SETNB:
			flagCond = FlagCond::NC;
			break;
		case ZYDIS_MNEMONIC_SETNBE:
			flagCond = FlagCond::A;
			break;
		case ZYDIS_MNEMONIC_SETNL:
			flagCond = FlagCond::NL;
			break;
		case ZYDIS_MNEMONIC_SETNLE:
			flagCond = FlagCond::NLE;
			break;
		case ZYDIS_MNEMONIC_SETNO:
			flagCond = FlagCond::O;
			break;
		case ZYDIS_MNEMONIC_SETNP:
			flagCond = FlagCond::NP;
			break;
		case ZYDIS_MNEMONIC_SETNS:
			flagCond = FlagCond::S;
			break;
		case ZYDIS_MNEMONIC_SETNZ:
			flagCond = FlagCond::NZ;
			break;
		case ZYDIS_MNEMONIC_SETO:
			flagCond = FlagCond::O;
			break;
		case ZYDIS_MNEMONIC_SETP:
			flagCond = FlagCond::P;
			break;
		case ZYDIS_MNEMONIC_SETS:
			flagCond = FlagCond::S;
			break;
		case ZYDIS_MNEMONIC_SETZ:
			flagCond = FlagCond::Z;
			break;
		}

		auto varnodeFlagCond = GetFlagCondition(flagCond);
		addGenericOperation(InstructionId::COPY, varnodeFlagCond, nullptr);
		break;
	}

	case ZYDIS_MNEMONIC_JB:
	case ZYDIS_MNEMONIC_JBE:
	case ZYDIS_MNEMONIC_JL:
	case ZYDIS_MNEMONIC_JLE:
	case ZYDIS_MNEMONIC_JNB:
	case ZYDIS_MNEMONIC_JNBE:
	case ZYDIS_MNEMONIC_JNL:
	case ZYDIS_MNEMONIC_JNLE:
	case ZYDIS_MNEMONIC_JNO:
	case ZYDIS_MNEMONIC_JNP:
	case ZYDIS_MNEMONIC_JNS:
	case ZYDIS_MNEMONIC_JNZ:
	case ZYDIS_MNEMONIC_JO:
	case ZYDIS_MNEMONIC_JP:
	case ZYDIS_MNEMONIC_JS:
	case ZYDIS_MNEMONIC_JZ:
	{
		FlagCond flagCond = FlagCond::NONE;
		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_JB:
			flagCond = FlagCond::C;
			break;
		case ZYDIS_MNEMONIC_JBE:
			flagCond = FlagCond::NA;
			break;
		case ZYDIS_MNEMONIC_JL:
			flagCond = FlagCond::L;
			break;
		case ZYDIS_MNEMONIC_JLE:
			flagCond = FlagCond::LE;
			break;
		case ZYDIS_MNEMONIC_JNB:
			flagCond = FlagCond::NC;
			break;
		case ZYDIS_MNEMONIC_JNBE:
			flagCond = FlagCond::A;
			break;
		case ZYDIS_MNEMONIC_JNL:
			flagCond = FlagCond::NL;
			break;
		case ZYDIS_MNEMONIC_JNLE:
			flagCond = FlagCond::NLE;
			break;
		case ZYDIS_MNEMONIC_JNO:
			flagCond = FlagCond::O;
			break;
		case ZYDIS_MNEMONIC_JNP:
			flagCond = FlagCond::NP;
			break;
		case ZYDIS_MNEMONIC_JNS:
			flagCond = FlagCond::S;
			break;
		case ZYDIS_MNEMONIC_JNZ:
			flagCond = FlagCond::NZ;
			break;
		case ZYDIS_MNEMONIC_JO:
			flagCond = FlagCond::O;
			break;
		case ZYDIS_MNEMONIC_JP:
			flagCond = FlagCond::P;
			break;
		case ZYDIS_MNEMONIC_JS:
			flagCond = FlagCond::S;
			break;
		case ZYDIS_MNEMONIC_JZ:
			flagCond = FlagCond::Z;
			break;
		}

		auto varnodeFlagCond = GetFlagCondition(flagCond);
		Varnode* varnodeInput0 = getJumpOffsetByOperand(m_curInstr->operands[0]);
		addMicroInstruction(InstructionId::CBRANCH, varnodeInput0, varnodeFlagCond);
		break;
	}

	default: {
		auto instr = addMicroInstruction(InstructionId::UNKNOWN, m_instrPool->createConstantVarnode(0x0, 0x8));
		getWarningContainer()->addWarning("impossible to decode " + m_curOrigInstr->m_originalView + " (at 0x" + Helper::String::NumberToHex(m_curOrigInstr->m_offset) + ")");
	}

	}
}

Varnode* CE::Decompiler::PCode::DecoderX86::getJumpOffsetByOperand(const ZydisDecodedOperand& operand) {
	if (operand.type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
		return requestOperandValue(operand, operand.size / 0x8);
	auto jmpOffset = (int)operand.imm.value.s;
	return m_instrPool->createConstantVarnode(getJumpOffset(jmpOffset), 0x8);
}

uint64_t CE::Decompiler::PCode::DecoderX86::getJumpOffset(int jmpOffset) const
{
	auto offset = m_curOrigInstr->m_offset + m_curOrigInstr->m_length + jmpOffset;
	return offset << 8;
}

void CE::Decompiler::PCode::DecoderX86::GenerateVectorOperation(const VectorOperationGeneratorInfo& info)
{
	int operationsCount = info.maxSize / info.size;
	auto instrId2 = info.instrId2;
	if (instrId2 == InstructionId::NONE) {
		instrId2 = info.instrId1;
	}

	auto& dstOperand = m_curInstr->operands[0];
	auto& srcOperand = m_curInstr->operands[1];

	for (int i = 0; i < operationsCount; i++)
		if (info.shuffOp2[i] != -1) {
			int idxOp1 = 0;
			int idxOp2 = 1;
			Varnode* ops[2] = { nullptr, nullptr };

			if (info.isOperationWithSingleOperand) {
				idxOp1 = idxOp2 = (i < operationsCount / 2) ? idxOp1 : idxOp2;
			}
			if (info.instrId1 == InstructionId::COPY || info.instrId1 == InstructionId::FLOAT_SQRT) {
				idxOp1 = idxOp2;
				idxOp2 = -1;
			}

			if (idxOp1 == 0 || idxOp2 == 0) {
				ops[0] = requestOperandValue(dstOperand, info.size, info.size * info.shuffOp1[i]);
			}
			if (idxOp1 == 1 || idxOp2 == 1) {
				ops[1] = requestOperandValue(srcOperand, info.size, info.size * info.shuffOp2[i]);
			}

			if (info.isNegate) {
				ops[idxOp1] = addGenericOperation(InstructionId::INT_NEGATE, ops[idxOp1], nullptr);
			}
			addGenericOperation(i % 2 ? info.instrId1 : instrId2, ops[idxOp1], idxOp2 != -1 ? ops[idxOp2] : nullptr, nullptr, false, info.size, info.size * i);
		}
}

Varnode* CE::Decompiler::PCode::DecoderX86::addGenericOperation(InstructionId instrId, Varnode* varnodeInput0, Varnode* varnodeInput1, Varnode* memLocVarnode, bool isFictitious, int size, int offset) {
	auto& operand = m_curInstr->operands[0];
	if (size == 0x0)
		size = operand.size / 0x8;
	Varnode* varnodeOutput = nullptr;
	if (!isFictitious && operand.type == ZYDIS_OPERAND_TYPE_REGISTER) {
		varnodeOutput = CreateVarnode(operand.reg.value, size, offset);
	}
	else {
		varnodeOutput = m_instrPool->createSymbolVarnode(size);
	}

	addMicroInstruction(instrId, varnodeInput0, varnodeInput1, varnodeOutput);

	if (!isFictitious && operand.type == ZYDIS_OPERAND_TYPE_MEMORY) {
		setDestinationMemOperand(operand, size, offset, varnodeOutput, memLocVarnode);
	}

	return varnodeOutput;
}

Instruction* CE::Decompiler::PCode::DecoderX86::addMicroInstruction(InstructionId id, Varnode* input0, Varnode* input1, Varnode* output, bool zext) {
	auto instr = m_instrPool->createInstruction(id, input0, input1, output, m_curOrigInstr, m_curOrderId++);
	if (m_curOrderId == 1) { //for debug info
		ZydisFormatter formatter;
		ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
		char buffer[256];
		ZydisFormatterFormatInstruction(&formatter, m_curInstr, buffer, sizeof(buffer),
			(ZyanU64)m_addr);
		m_curOrigInstr->m_originalView = buffer;
	}
	m_result.push_back(instr);
	return instr;
}

void CE::Decompiler::PCode::DecoderX86::setDestinationMemOperand(const ZydisDecodedOperand& operand, int size, int offset, Varnode* varnode, Varnode* memLocVarnode) {
	if (!memLocVarnode) {
		memLocVarnode = requestOperandValue(operand, size, offset, nullptr, false);
	}
	addMicroInstruction(InstructionId::STORE, memLocVarnode, varnode);
}

Varnode* CE::Decompiler::PCode::DecoderX86::requestOperandValue(const ZydisDecodedOperand& operand, int size, Varnode** memLocVarnode, bool isMemLocLoaded, int memLocExprSize) {
	return requestOperandValue(operand, size, 0x0, memLocVarnode, isMemLocLoaded, memLocExprSize);
}

Varnode* CE::Decompiler::PCode::DecoderX86::requestOperandValue(const ZydisDecodedOperand& operand, int size, int offset, Varnode** memLocVarnode, bool isMemLocLoaded, int memLocExprSize) {
	if (operand.type == ZYDIS_OPERAND_TYPE_REGISTER) {
		return CreateVarnode(operand.reg.value, size, offset);
	}

	if (operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
		return m_instrPool->createConstantVarnode(operand.imm.value.u & BitMask64(size).getValue(), size);
	}

	if (operand.type == ZYDIS_OPERAND_TYPE_MEMORY) {
		Varnode* resultVarnode = nullptr;
		RegisterVarnode* baseRegVarnode = nullptr;
		RegisterVarnode* indexRegVarnode = nullptr;

		if (operand.mem.base != ZYDIS_REGISTER_NONE) {
			baseRegVarnode = CreateVarnode(operand.mem.base, memLocExprSize);
			if (operand.mem.base == ZYDIS_REGISTER_RIP) {
				offset += m_curOrigInstr->m_offset + m_curOrigInstr->m_length;
			}
		}

		if (operand.mem.index != ZYDIS_REGISTER_NONE) {
			resultVarnode = indexRegVarnode = CreateVarnode(operand.mem.index, memLocExprSize);
			if (operand.mem.scale != 1) {
				auto symbolVarnode = m_instrPool->createSymbolVarnode(memLocExprSize);
				addMicroInstruction(InstructionId::INT_MULT, resultVarnode, m_instrPool->createConstantVarnode(operand.mem.scale & resultVarnode->getMask().getValue(), memLocExprSize), symbolVarnode);
				resultVarnode = symbolVarnode;
			}
			if (baseRegVarnode != nullptr) {
				auto symbolVarnode = m_instrPool->createSymbolVarnode(memLocExprSize);
				addMicroInstruction(InstructionId::INT_ADD, baseRegVarnode, resultVarnode, symbolVarnode);
				resultVarnode = symbolVarnode;
			}
		}
		else {
			resultVarnode = baseRegVarnode;
		}

		if (operand.mem.disp.has_displacement) {
			auto constValue = (uint64_t&)operand.mem.disp.value & resultVarnode->getMask().getValue();
			auto dispVarnode = m_instrPool->createConstantVarnode(constValue, memLocExprSize);
			if (resultVarnode != nullptr) {
				auto symbolVarnode = m_instrPool->createSymbolVarnode(memLocExprSize);
				addMicroInstruction(InstructionId::INT_ADD, resultVarnode, dispVarnode, symbolVarnode);
				resultVarnode = symbolVarnode;
			}
			else {
				resultVarnode = dispVarnode;
			}
		}

		if (offset > 0) {
			auto symbolVarnode = m_instrPool->createSymbolVarnode(memLocExprSize);
			addMicroInstruction(InstructionId::INT_ADD, resultVarnode, m_instrPool->createConstantVarnode(offset, memLocExprSize), symbolVarnode);
			resultVarnode = symbolVarnode;
		}

		if (memLocVarnode) {
			*memLocVarnode = resultVarnode;
		}

		if (isMemLocLoaded) { //check for LEA instruction
			auto symbolVarnode = m_instrPool->createSymbolVarnode(size);
			addMicroInstruction(InstructionId::LOAD, resultVarnode, nullptr, symbolVarnode);
			resultVarnode = symbolVarnode;
		}
		return resultVarnode;
	}
	return nullptr;
}

Varnode* CE::Decompiler::PCode::DecoderX86::GetFlagCondition(FlagCond flagCond) {
	Varnode* varnodeCond = nullptr;

	switch (flagCond)
	{
	case FlagCond::Z:
	case FlagCond::NZ:
	case FlagCond::C:
	case FlagCond::NC:
	case FlagCond::P:
	case FlagCond::NP:
	case FlagCond::O:
	case FlagCond::NO:
	case FlagCond::S:
	case FlagCond::NS:
	{
		auto flag = ZYDIS_CPUFLAG_ZF;
		switch (flagCond)
		{
		case FlagCond::C:
		case FlagCond::NC:
			flag = ZYDIS_CPUFLAG_CF;
			break;
		case FlagCond::P:
		case FlagCond::NP:
			flag = ZYDIS_CPUFLAG_PF;
			break;
		case FlagCond::O:
		case FlagCond::NO:
			flag = ZYDIS_CPUFLAG_OF;
			break;
		case FlagCond::S:
		case FlagCond::NS:
			flag = ZYDIS_CPUFLAG_SF;
			break;
		}

		varnodeCond = CreateVarnode(flag);
		if (flagCond == FlagCond::NZ || flagCond == FlagCond::NC || flagCond == FlagCond::NP || flagCond == FlagCond::NO) {
			auto varnodeNeg = m_instrPool->createSymbolVarnode(1);
			addMicroInstruction(InstructionId::BOOL_NEGATE, varnodeCond, nullptr, varnodeNeg);
			varnodeCond = varnodeNeg;
		}
		break;
	}
	case FlagCond::L:
	case FlagCond::LE:
	{
		auto varnodeNe = m_instrPool->createSymbolVarnode(1);
		addMicroInstruction(InstructionId::INT_NOTEQUAL, CreateVarnode(ZYDIS_CPUFLAG_OF), CreateVarnode(ZYDIS_CPUFLAG_SF), varnodeNe);
		if (flagCond == FlagCond::LE) {
			auto varnodeOr = m_instrPool->createSymbolVarnode(1);
			addMicroInstruction(InstructionId::BOOL_OR, CreateVarnode(ZYDIS_CPUFLAG_ZF), varnodeNe, varnodeOr);
			varnodeCond = varnodeOr;
		}
		else {
			varnodeCond = varnodeNe;
		}
		break;
	}
	case FlagCond::NL:
	case FlagCond::NLE:
	{
		auto varnodeEq = m_instrPool->createSymbolVarnode(1);
		addMicroInstruction(InstructionId::INT_EQUAL, CreateVarnode(ZYDIS_CPUFLAG_OF), CreateVarnode(ZYDIS_CPUFLAG_SF), varnodeEq);
		if (flagCond == FlagCond::NLE) {
			auto varnodeNeg = m_instrPool->createSymbolVarnode(1);
			addMicroInstruction(InstructionId::BOOL_NEGATE, CreateVarnode(ZYDIS_CPUFLAG_ZF), nullptr, varnodeNeg);
			auto varnodeAnd = m_instrPool->createSymbolVarnode(1);
			addMicroInstruction(InstructionId::BOOL_AND, varnodeEq, varnodeNeg, varnodeAnd);
			varnodeCond = varnodeAnd;
		}
		else {
			varnodeCond = varnodeEq;
		}
		break;
	}
	case FlagCond::A:
	case FlagCond::NA:
	{
		auto varnodeOr = m_instrPool->createSymbolVarnode(1);
		addMicroInstruction(InstructionId::BOOL_OR, CreateVarnode(ZYDIS_CPUFLAG_CF), CreateVarnode(ZYDIS_CPUFLAG_ZF), varnodeOr);
		if (flagCond == FlagCond::A) {
			auto varnodeNe = m_instrPool->createSymbolVarnode(1);
			addMicroInstruction(InstructionId::BOOL_NEGATE, varnodeOr, nullptr, varnodeNe);
			varnodeCond = varnodeNe;
		}
		else {
			varnodeCond = varnodeOr;
		}
		break;
	}
	}

	return varnodeCond;
}

int CE::Decompiler::PCode::DecoderX86::getFirstExplicitOperandsCount() const
{
	int result = 0;
	for (int i = 0; i < m_curInstr->operand_count; i++) {
		if (m_curInstr->operands[result].visibility != ZYDIS_OPERAND_VISIBILITY_HIDDEN)
			result++;
	}
	return result;
}

RegisterVarnode* CE::Decompiler::PCode::DecoderX86::CreateVarnode(ZydisRegister regId, int size, int offset) const
{
	auto reg = m_registerFactoryX86->createRegister(regId, size, offset);
	return m_instrPool->createRegisterVarnode(reg);
}

RegisterVarnode* CE::Decompiler::PCode::DecoderX86::CreateVarnode(ZydisCPUFlag flag) const
{
	return m_instrPool->createRegisterVarnode(m_registerFactoryX86->createFlagRegister(flag));
}
