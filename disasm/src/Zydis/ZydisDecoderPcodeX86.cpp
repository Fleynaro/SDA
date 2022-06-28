#include "Disasm/Zydis/ZydisDecoderPcodeX86.h"
#include <sstream>
#include <iomanip>

using namespace sda;
using namespace sda::pcode;
using namespace sda::disasm;

ZydisDecoderPcodeX86::ZydisDecoderPcodeX86(ZydisDecoder* decoder)
    : m_decoder(decoder)
{}

void ZydisDecoderPcodeX86::decode(Offset offset, const std::vector<uint8_t>& data) {
    auto status = ZydisDecoderDecodeFull(
        m_decoder,
        data.data(),
        data.size(),
        &m_curInstr,
        m_curOperands,
        ZYDIS_MAX_OPERAND_COUNT_VISIBLE, 
        ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY);

    if (ZYAN_SUCCESS(status)) {
        m_curOffset = offset;
		m_curInstrIndex = 0;
        generatePcodeInstructions();
    }
}

size_t ZydisDecoderPcodeX86::getInstructionLength() const {
	return m_curInstr.length;
}

void ZydisDecoderPcodeX86::generatePcodeInstructions() {
    auto mnemonic = m_curInstr.mnemonic;
	auto size = m_curOperands[0].size / 0x8;
	auto explicitOperandsCount = getFirstExplicitOperandsCount();

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
        auto input0 = getRegisterVarnode(ZYDIS_REGISTER_RAX, srcRegSize);
        auto output = getRegisterVarnode(ZYDIS_REGISTER_RAX, srcRegSize * 2);
		generateInstruction(InstructionId::INT_SEXT, input0, nullptr, output);
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

		auto varnode = getVirtRegisterVarnode(srcRegSize * 2);
        auto raxVarnode = getRegisterVarnode(ZYDIS_REGISTER_RAX, srcRegSize);
        auto rdxVarnode = getRegisterVarnode(ZYDIS_REGISTER_RDX, srcRegSize);
        auto sizeVarnode = getConstantVarnode(srcRegSize, 0x4);

		generateInstruction(InstructionId::INT_SEXT, raxVarnode, nullptr, varnode);
		generateInstruction(InstructionId::SUBPIECE, varnode, sizeVarnode, rdxVarnode);
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
			instrId = InstructionId::FLOAT_ROUND;
			dstSize = 0x4;
			srcSize = 0x4;
			break;
		case ZYDIS_MNEMONIC_CVTSD2SI:
			instrId = InstructionId::FLOAT_ROUND;
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
			instrId = InstructionId::FLOAT_ROUND;
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
		auto& dstOperand = m_curOperands[0];
		auto& srcOperand = m_curOperands[1];
		while (offset < maxSize) {
			auto srcOpVarnode = getOperandVarnode(srcOperand, srcSize, offset);
			auto varnodeRegOutput = getRegisterVarnode(dstOperand.reg.value, dstSize, offset);
			if (float2intForNonVector && varnodeRegOutput->getRegType() != RegisterVarnode::Vector) {
				//all float values store in vector registers then need cast when moving to non-vector register
				auto varnodeOutput = getVirtRegisterVarnode(size);
				generateInstruction(instrId, srcOpVarnode, nullptr, varnodeOutput);
				generateInstruction(InstructionId::FLOAT2INT, varnodeOutput, nullptr, varnodeRegOutput);
			}
			else {
				generateInstruction(instrId, srcOpVarnode, nullptr, varnodeRegOutput);
			}
			offset += std::max(dstSize, srcSize);
			offset2 += srcSize;
		}

		while (offset2 < maxSize) {
			auto varnodeRegOutput = getRegisterVarnode(dstOperand.reg.value, dstSize, offset);
            auto zero = getConstantVarnode(0x0, dstSize);
			generateInstruction(InstructionId::COPY, zero, nullptr, varnodeRegOutput);
			offset2 += srcSize;
		}
		break;
	}

	case ZYDIS_MNEMONIC_COMISD:
	case ZYDIS_MNEMONIC_COMISS:
	case ZYDIS_MNEMONIC_UCOMISD:
	case ZYDIS_MNEMONIC_UCOMISS:
	{
		auto op1Varnode = getOperandVarnode(m_curOperands[0], size);
		auto op2Varnode = getOperandVarnode(m_curOperands[1], size);

		auto varnodeNan1 = getVirtRegisterVarnode(1);
		generateInstruction(InstructionId::FLOAT_NAN, op1Varnode, nullptr, varnodeNan1);

		auto varnodeNan2 = getVirtRegisterVarnode(1);
		generateInstruction(InstructionId::FLOAT_NAN, op2Varnode, nullptr, varnodeNan2);

		auto flagPF = getRegisterVarnode(ZYDIS_CPUFLAG_PF);
		generateInstruction(InstructionId::BOOL_OR, varnodeNan1, varnodeNan2, flagPF);

		auto varnodeEq = getVirtRegisterVarnode(1);
		generateInstruction(InstructionId::FLOAT_EQUAL, op1Varnode, op2Varnode, varnodeEq);
		generateInstruction(InstructionId::BOOL_OR, flagPF, varnodeEq, getRegisterVarnode(ZYDIS_CPUFLAG_ZF));

		auto varnodeFl = getVirtRegisterVarnode(1);
		generateInstruction(InstructionId::FLOAT_LESS, op1Varnode, op2Varnode, varnodeFl);
		generateInstruction(InstructionId::BOOL_OR, flagPF, varnodeFl, getRegisterVarnode(ZYDIS_CPUFLAG_CF));

		auto zeroVanrnode = getConstantVarnode(0x0, 1);
		generateInstruction(InstructionId::COPY, zeroVanrnode, nullptr, getRegisterVarnode(ZYDIS_CPUFLAG_OF));
		generateInstruction(InstructionId::COPY, zeroVanrnode, nullptr, getRegisterVarnode(ZYDIS_CPUFLAG_AF));
		generateInstruction(InstructionId::COPY, zeroVanrnode, nullptr, getRegisterVarnode(ZYDIS_CPUFLAG_SF));
		break;
	}

	case ZYDIS_MNEMONIC_MOVD:
	case ZYDIS_MNEMONIC_MOVSS:
	{
		auto& dstOperand = m_curOperands[0];
		auto& srcOperand = m_curOperands[1];
		auto srcOpVarnode = getOperandVarnode(srcOperand, 0x4, 0x0);
		generateGenericOperation(InstructionId::COPY, srcOpVarnode, nullptr, nullptr, false, 0x4, 0x0);

		if (mnemonic == ZYDIS_MNEMONIC_MOVD && dstOperand.type == ZYDIS_OPERAND_TYPE_REGISTER ||
            mnemonic == ZYDIS_MNEMONIC_MOVSS && srcOperand.type == ZYDIS_OPERAND_TYPE_MEMORY) {
			auto zero = getConstantVarnode(0x0, 0x4);
			generateGenericOperation(InstructionId::COPY, zero, nullptr, nullptr, false, 0x4, 4);
			if (size >= 0x10) {
				generateGenericOperation(InstructionId::COPY, zero, nullptr, nullptr, false, 0x4, 8);
				generateGenericOperation(InstructionId::COPY, zero, nullptr, nullptr, false, 0x4, 12);
			}
		}
		break;
	}

	case ZYDIS_MNEMONIC_MOVQ:
	case ZYDIS_MNEMONIC_MOVSD:
	{
		auto& dstOperand = m_curOperands[0];
		auto& srcOperand = m_curOperands[1];
		auto srcOpVarnode = getOperandVarnode(srcOperand, 0x8, 0x0);
		generateGenericOperation(InstructionId::COPY, srcOpVarnode, nullptr, nullptr, false, 0x8, 0x0);
		if (mnemonic == ZYDIS_MNEMONIC_MOVQ && dstOperand.type != ZYDIS_OPERAND_TYPE_MEMORY) {
			if (size >= 0x10) {
                auto zero = getConstantVarnode(0x0, 0x8);
				generateGenericOperation(InstructionId::COPY, zero, nullptr, nullptr, false, 0x8, 0x8);
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
		auto& dstOperand = m_curOperands[0];
		auto& srcOperand = m_curOperands[1];
		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_UNPCKHPD: {
			auto dstOpVarnode = getOperandVarnode(dstOperand, 0x8, 0x8);
			auto srcOpVarnode = getOperandVarnode(srcOperand, 0x8, 0x8);
			generateGenericOperation(InstructionId::COPY, dstOpVarnode, nullptr, nullptr, false, 0x8, 0x0);
			generateGenericOperation(InstructionId::COPY, srcOpVarnode, nullptr, nullptr, false, 0x8, 0x8);
			break;
		}
		case ZYDIS_MNEMONIC_UNPCKHPS: {
			auto dstOpVarnode1 = getOperandVarnode(dstOperand, 0x4, 8);
			auto srcOpVarnode1 = getOperandVarnode(srcOperand, 0x4, 8);
			auto dstOpVarnode2 = getOperandVarnode(dstOperand, 0x4, 12);
			auto srcOpVarnode2 = getOperandVarnode(srcOperand, 0x4, 12);
			generateGenericOperation(InstructionId::COPY, dstOpVarnode1, nullptr, nullptr, false, 0x4, 0);
			generateGenericOperation(InstructionId::COPY, srcOpVarnode1, nullptr, nullptr, false, 0x4, 4);
			generateGenericOperation(InstructionId::COPY, dstOpVarnode2, nullptr, nullptr, false, 0x4, 8);
			generateGenericOperation(InstructionId::COPY, srcOpVarnode2, nullptr, nullptr, false, 0x4, 12);
			break;
		}
		case ZYDIS_MNEMONIC_UNPCKLPD: {
			auto srcOpVarnode = getOperandVarnode(srcOperand, 0x8, 0x0);
			generateGenericOperation(InstructionId::COPY, srcOpVarnode, nullptr, nullptr, false, 0x8, 0x8);
			break;
		}
		case ZYDIS_MNEMONIC_UNPCKLPS: {
			auto dstOpVarnode = getOperandVarnode(dstOperand, 0x4, 4);
			auto srcOpVarnode1 = getOperandVarnode(srcOperand, 0x4, 4);
			auto srcOpVarnode2 = getOperandVarnode(srcOperand, 0x4, 0);
			generateGenericOperation(InstructionId::COPY, dstOpVarnode, nullptr, nullptr, false, 0x4, 8);
			generateGenericOperation(InstructionId::COPY, srcOpVarnode1, nullptr, nullptr, false, 0x4, 12);
			generateGenericOperation(InstructionId::COPY, srcOpVarnode2, nullptr, nullptr, false, 0x4, 4);
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
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_MOVUPD);
		}
		else if (ZYDIS_MNEMONIC_MOVAPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_MOVAPS) {
			info.instrId1 = InstructionId::COPY;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_MOVAPD);
		}
		else if (ZYDIS_MNEMONIC_BLENDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_BLENDPS) {
			info.instrId1 = InstructionId::COPY;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_BLENDPD);
			auto& infoOperand = m_curOperands[2];
			for (int i = 0; i < 4; i++) {
				info.shuffOp2[i] = ((infoOperand.imm.value.u >> i) & 0b1) ? info.shuffOp2[i] : -1;
			}
		}
		else if (ZYDIS_MNEMONIC_SHUFPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_SHUFPS) {
			info.instrId1 = InstructionId::COPY;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_SHUFPD);
			info.isOperationWithSingleOperand = true;
			auto& infoOperand = m_curOperands[2];
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
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_ADDSUBPD);
		}
		else if (ZYDIS_MNEMONIC_HADDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_HADDPS ||
                    ZYDIS_MNEMONIC_HSUBPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_HSUBPS) {
			if (ZYDIS_MNEMONIC_HADDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_HADDPS) {
				info.instrId1 = InstructionId::INT_ADD;
				operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_HADDPD);
			}
			else {
				info.instrId1 = InstructionId::INT_SUB;
				operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_HSUBPD);
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
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_HSUBPD);
		}
		else if (ZYDIS_MNEMONIC_ADDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_ADDSS) {
			info.instrId1 = InstructionId::FLOAT_ADD;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_ADDPD);
		}
		else if (ZYDIS_MNEMONIC_SUBPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_SUBSS) {
			info.instrId1 = InstructionId::FLOAT_SUB;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_SUBPD);
		}
		else if (ZYDIS_MNEMONIC_MULPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_MULSS) {
			info.instrId1 = InstructionId::FLOAT_MULT;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_MULPD);
		}
		else if (ZYDIS_MNEMONIC_DIVPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_DIVSS) {
			info.instrId1 = InstructionId::FLOAT_DIV;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_DIVPD);
		}
		else if (ZYDIS_MNEMONIC_ANDPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_ANDPS) {
			info.instrId1 = InstructionId::INT_AND;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_ANDPD);
		}
		else if (ZYDIS_MNEMONIC_ANDNPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_ANDNPS) {
			info.instrId1 = InstructionId::INT_AND;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_ANDNPD);
			info.isNegate = true;
		}
		else if (ZYDIS_MNEMONIC_ORPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_ORPS) {
			info.instrId1 = InstructionId::INT_OR;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_ORPD);
		}
		else if (ZYDIS_MNEMONIC_XORPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_XORPS) {
			info.instrId1 = InstructionId::INT_XOR;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_XORPD);
		}
		else if (ZYDIS_MNEMONIC_SQRTPD <= mnemonic && mnemonic <= ZYDIS_MNEMONIC_SQRTSS) {
			info.instrId1 = InstructionId::FLOAT_SQRT;
			operationSize = static_cast<OperationSize>(mnemonic - ZYDIS_MNEMONIC_SQRTPD);
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
		generateVectorOperation(info);
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
			auto& infoOperand = m_curOperands[2];
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
				info.shuffOp1[startIdx + i] =
                info.shuffOp2[startIdx + i] = startIdx + ((infoOperand.imm.value.u >> (i * 2)) & 0b11);
			}
			break;
		}
		}

		generateVectorOperation(info);
		break;
	}

	case ZYDIS_MNEMONIC_XCHG:
	{
		auto varnodeInput0 = getOperandVarnode(m_curOperands[0], size);
		auto varnodeInput1 = getOperandVarnode(m_curOperands[1], size);
		auto varnodeTemp = getVirtRegisterVarnode(size);
		generateInstruction(InstructionId::COPY, varnodeInput0, nullptr, varnodeTemp);
		generateInstruction(InstructionId::COPY, varnodeInput1, nullptr, varnodeInput0);
		generateInstruction(InstructionId::COPY, varnodeTemp, nullptr, varnodeInput1);
		break;
	}

	case ZYDIS_MNEMONIC_MOV:
	case ZYDIS_MNEMONIC_MOVZX:
	case ZYDIS_MNEMONIC_MOVSX:
	case ZYDIS_MNEMONIC_MOVSXD:
	case ZYDIS_MNEMONIC_LEA:
	{
		auto& operand = m_curOperands[1];
		std::shared_ptr<Varnode> varnode;
		if (mnemonic == ZYDIS_MNEMONIC_LEA) {
			varnode = getOperandVarnode(operand, operand.size / 0x8, nullptr, false, size);
		}
		else {
			varnode = getOperandVarnode(operand, operand.size / 0x8, nullptr);
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
		generateGenericOperation(instrId, varnode, nullptr);
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
		std::shared_ptr<Varnode> memLocVarnode = nullptr;
		std::shared_ptr<Varnode> varnodeInput0 = getOperandVarnode(m_curOperands[0], size, &memLocVarnode);
		std::shared_ptr<Varnode> varnodeInput1 = nullptr;
		std::shared_ptr<Varnode> varnodeInput2 = nullptr;
		std::shared_ptr<Varnode> varnodeOutput = nullptr;
		if (explicitOperandsCount >= 2)
			varnodeInput1 = getOperandVarnode(m_curOperands[1], size);
		if (explicitOperandsCount >= 3)
			varnodeInput2 = getOperandVarnode(m_curOperands[2], size);

		switch (mnemonic)
		{
		case ZYDIS_MNEMONIC_ADD:
		case ZYDIS_MNEMONIC_INC:
		case ZYDIS_MNEMONIC_DEC:
		{
			auto flagOfInstrId = InstructionId::INT_SCARRY;
			if (mnemonic == ZYDIS_MNEMONIC_INC || mnemonic == ZYDIS_MNEMONIC_DEC) {
				varnodeInput1 = getConstantVarnode(0x1, size);
				if (mnemonic == ZYDIS_MNEMONIC_DEC) {
					flagOfInstrId = InstructionId::INT_SBORROW;
				}
			}
			else {
				generateInstruction(InstructionId::INT_CARRY, varnodeInput0, varnodeInput1, getRegisterVarnode(ZYDIS_CPUFLAG_CF));
			}
			generateInstruction(flagOfInstrId, varnodeInput0, varnodeInput1, getRegisterVarnode(ZYDIS_CPUFLAG_OF));
            auto instrId = mnemonic == ZYDIS_MNEMONIC_DEC ? InstructionId::INT_SUB : InstructionId::INT_ADD;
			varnodeOutput = generateGenericOperation(instrId, varnodeInput0, varnodeInput1, memLocVarnode);
			break;
		}

		case ZYDIS_MNEMONIC_SUB:
		case ZYDIS_MNEMONIC_CMP:
		{
			generateInstruction(InstructionId::INT_LESS, varnodeInput0, varnodeInput1, getRegisterVarnode(ZYDIS_CPUFLAG_CF));
			generateInstruction(InstructionId::INT_SBORROW, varnodeInput0, varnodeInput1, getRegisterVarnode(ZYDIS_CPUFLAG_OF));
			varnodeOutput = generateGenericOperation(InstructionId::INT_SUB, varnodeInput0, varnodeInput1, memLocVarnode, mnemonic == ZYDIS_MNEMONIC_CMP);
			break;
		}

		case ZYDIS_MNEMONIC_NEG:
		{
			auto varnodeZero = getConstantVarnode(0x0, size);
			generateInstruction(InstructionId::INT_NOTEQUAL, varnodeInput0, varnodeZero, getRegisterVarnode(ZYDIS_CPUFLAG_CF));
			generateInstruction(InstructionId::INT_SBORROW, varnodeZero, varnodeInput0, getRegisterVarnode(ZYDIS_CPUFLAG_OF));
			varnodeOutput = generateGenericOperation(InstructionId::INT_2COMP, varnodeInput0, nullptr, memLocVarnode);
			break;
		}

		case ZYDIS_MNEMONIC_MUL:
		case ZYDIS_MNEMONIC_IMUL:
		{
			std::shared_ptr<Varnode> varnodeDst = nullptr;
			std::shared_ptr<Varnode> varnodeMul1;
			std::shared_ptr<Varnode> varnodeMul2;
			std::shared_ptr<Varnode> varnodeCF = getRegisterVarnode(ZYDIS_CPUFLAG_CF);

			if (explicitOperandsCount == 1) {
				varnodeDst = varnodeMul1 = getRegisterVarnode(ZYDIS_REGISTER_RAX, size);
				varnodeMul2 = varnodeInput0;
			}
			else if (explicitOperandsCount == 2) {
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

			auto varnodeZext1 = getVirtRegisterVarnode(size * 2);
			generateInstruction(instrExt, varnodeMul1, nullptr, varnodeZext1);
			auto varnodeZext2 = getVirtRegisterVarnode(size * 2);
			generateInstruction(instrExt, varnodeMul2, nullptr, varnodeZext2);
			auto varnodeMult = getVirtRegisterVarnode(size * 2);
			generateInstruction(InstructionId::INT_MULT, varnodeZext1, varnodeZext2, varnodeMult);

			std::shared_ptr<Varnode> varnodeSubpiece;
			if (explicitOperandsCount == 1) {
				varnodeSubpiece = getRegisterVarnode(ZYDIS_REGISTER_RDX, size);
			}
			else {
				varnodeSubpiece = getVirtRegisterVarnode(size * 2);
			}
			if (mnemonic == ZYDIS_MNEMONIC_IMUL) {
				generateInstruction(InstructionId::INT_MULT, varnodeMul1, varnodeMul2, varnodeDst);
				generateInstruction(InstructionId::SUBPIECE, varnodeMult, getConstantVarnode(size, 0x4), varnodeSubpiece);
				auto varnodeNe1 = getVirtRegisterVarnode(0x1);
				generateInstruction(InstructionId::INT_NOTEQUAL, varnodeSubpiece, getConstantVarnode(0x0, size), varnodeNe1);
				auto varnode2Cmp = getVirtRegisterVarnode(size);
				generateInstruction(InstructionId::INT_2COMP, getConstantVarnode(0x1, size), nullptr, varnode2Cmp);
				auto varnodeNe2 = getVirtRegisterVarnode(0x1);
				generateInstruction(InstructionId::INT_NOTEQUAL, varnodeSubpiece, varnode2Cmp, varnodeNe2);
				generateInstruction(InstructionId::INT_AND, varnodeNe1, varnodeNe2, varnodeCF);
			}
			else {
				generateInstruction(InstructionId::SUBPIECE, varnodeMult, getConstantVarnode(size, 0x4), varnodeSubpiece);
				generateInstruction(InstructionId::INT_MULT, varnodeMul1, varnodeMul2, varnodeDst); //not like ghidra
				generateInstruction(InstructionId::INT_NOTEQUAL, varnodeSubpiece, getConstantVarnode(0x0, size), varnodeCF);
			}

			generateInstruction(InstructionId::COPY, varnodeCF, nullptr, getRegisterVarnode(ZYDIS_CPUFLAG_OF));
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

			auto varnodeRax = getRegisterVarnode(ZYDIS_REGISTER_RAX, size);
			auto varnodeRdx = getRegisterVarnode(ZYDIS_REGISTER_RDX, size);
			auto varnodeExt = getVirtRegisterVarnode(size * 2);
			generateInstruction(instrExt, varnodeInput0, nullptr, varnodeExt);

			auto varnodeZext1 = getVirtRegisterVarnode(size * 2);
			generateInstruction(InstructionId::INT_ZEXT, varnodeRdx, nullptr, varnodeZext1);
			auto varnodeLeft = getVirtRegisterVarnode(size * 2);
			generateInstruction(InstructionId::INT_LEFT, varnodeZext1, getConstantVarnode(size * 0x8, 0x4), varnodeLeft);
			auto varnodeZext2 = getVirtRegisterVarnode(size * 2);
			generateInstruction(InstructionId::INT_ZEXT, varnodeRax, nullptr, varnodeZext2);
			auto varnodeOr = getVirtRegisterVarnode(size * 2);
			generateInstruction(InstructionId::INT_OR, varnodeLeft, varnodeZext2, varnodeOr);

			auto varnodeDiv = getVirtRegisterVarnode(size * 2);
			generateInstruction(instrDiv, varnodeOr, varnodeExt, varnodeDiv);

			generateInstruction(InstructionId::SUBPIECE, varnodeDiv, getConstantVarnode(0x0, 0x4), varnodeRax);
			auto varnodeRem = getVirtRegisterVarnode(size * 2);
			generateInstruction(instrRem, varnodeOr, varnodeExt, varnodeRem);
			generateInstruction(InstructionId::SUBPIECE, varnodeRem, getConstantVarnode(0x0, 0x4), varnodeRdx);
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
				varnodeInput0 = generateGenericOperation(InstructionId::INT_NEGATE, varnodeInput0, nullptr);
			}
			generateInstruction(InstructionId::COPY, getConstantVarnode(0x0, size), nullptr, getRegisterVarnode(ZYDIS_CPUFLAG_CF));
			generateInstruction(InstructionId::COPY, getConstantVarnode(0x0, size), nullptr, getRegisterVarnode(ZYDIS_CPUFLAG_OF));
			varnodeOutput = generateGenericOperation(instrId, varnodeInput0, varnodeInput1, memLocVarnode, mnemonic == ZYDIS_MNEMONIC_TEST);
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
			auto varnodeAndInput1 = getVirtRegisterVarnode(0x8);
			generateInstruction(InstructionId::INT_AND, varnodeInput1, getConstantVarnode(63, size), varnodeAndInput1);
			generateGenericOperation(instrId, varnodeInput0, varnodeAndInput1, memLocVarnode);
			//flags ...
			break;
		}

		case ZYDIS_MNEMONIC_BT:
		case ZYDIS_MNEMONIC_BTR:
		{
			auto varnodeAndInput1 = getVirtRegisterVarnode(0x8);
			generateInstruction(InstructionId::INT_AND, varnodeInput1, getConstantVarnode(63, size), varnodeAndInput1);
			auto varnodeRight = getVirtRegisterVarnode(0x8);
			generateInstruction(InstructionId::INT_RIGHT, varnodeInput0, varnodeAndInput1, varnodeRight);
			auto varnodeAnd = getVirtRegisterVarnode(0x8);
			generateInstruction(InstructionId::INT_AND, varnodeRight, getConstantVarnode(1, size), varnodeAnd);

			if (mnemonic != ZYDIS_MNEMONIC_BT) {
				auto varnodeLeft = getVirtRegisterVarnode(0x8);
				generateInstruction(InstructionId::INT_LEFT, getConstantVarnode(1, size), varnodeAndInput1, varnodeLeft);
				auto varnodeNegate = getVirtRegisterVarnode(0x8);
				generateInstruction(InstructionId::INT_NEGATE, varnodeLeft, nullptr, varnodeNegate);
				generateGenericOperation(InstructionId::INT_AND, varnodeInput0, varnodeNegate, memLocVarnode);
			}

			generateInstruction(InstructionId::INT_NOTEQUAL, varnodeAnd, getConstantVarnode(0x0, size), getRegisterVarnode(ZYDIS_CPUFLAG_CF));
			break;
		}
		}

		if (varnodeOutput) {
			generateInstruction(InstructionId::INT_SLESS, varnodeOutput, getConstantVarnode(0x0, size), getRegisterVarnode(ZYDIS_CPUFLAG_SF));
			generateInstruction(InstructionId::INT_EQUAL, varnodeOutput, getConstantVarnode(0x0, size), getRegisterVarnode(ZYDIS_CPUFLAG_ZF));
		}
		break;
	}

	case ZYDIS_MNEMONIC_NOT:
	{
		std::shared_ptr<Varnode> memLocVarnode = nullptr;
		auto varnodeInput0 = getOperandVarnode(m_curOperands[0], size, &memLocVarnode);
		generateGenericOperation(InstructionId::INT_NEGATE, varnodeInput0, nullptr, memLocVarnode);
		break;
	}

	case ZYDIS_MNEMONIC_PUSH:
	{
		auto varnodeReg = getOperandVarnode(m_curOperands[0], size);
		auto varnodeRsp = getRegisterVarnode(ZYDIS_REGISTER_RSP, 0x8);
		generateInstruction(InstructionId::INT_SUB, varnodeRsp, getConstantVarnode(size, 0x8), varnodeRsp);
		generateInstruction(InstructionId::STORE, varnodeRsp, varnodeReg);
		break;
	}

	case ZYDIS_MNEMONIC_POP:
	{
		auto varnodeReg = getOperandVarnode(m_curOperands[0], size);
		auto varnodeRsp = getRegisterVarnode(ZYDIS_REGISTER_RSP, 0x8);
		generateInstruction(InstructionId::LOAD, varnodeRsp, nullptr, varnodeReg);
		generateInstruction(InstructionId::INT_ADD, varnodeRsp, getConstantVarnode(size, 0x8), varnodeRsp);
		break;
	}

	case ZYDIS_MNEMONIC_INT:
	case ZYDIS_MNEMONIC_INT3: {
		auto varnodeRip = getRegisterVarnode(ZYDIS_REGISTER_RIP, 0x8);
		generateInstruction(InstructionId::INT, varnodeRip, nullptr);
		break;
	}

	case ZYDIS_MNEMONIC_RET:
	{
		auto varnodeRip = getRegisterVarnode(ZYDIS_REGISTER_RIP, 0x8);
		auto varnodeRsp = getRegisterVarnode(ZYDIS_REGISTER_RSP, 0x8);
		generateInstruction(InstructionId::LOAD, varnodeRsp, nullptr, varnodeRip);
		generateInstruction(InstructionId::INT_ADD, varnodeRsp, getConstantVarnode(size, 0x8), varnodeRsp);
		generateInstruction(InstructionId::RETURN, varnodeRip, nullptr);
		break;
	}

	case ZYDIS_MNEMONIC_JMP:
	case ZYDIS_MNEMONIC_CALL:
	{
		auto varnodeInput0 = getJumpOffsetByOperand(m_curOperands[0]);
		if (mnemonic == ZYDIS_MNEMONIC_JMP) {
			generateInstruction(InstructionId::BRANCH, varnodeInput0, nullptr);
		}
		else {
			/*if (false) {
			auto varnodeRsp = getRegisterVarnode(ZYDIS_REGISTER_RSP, 0x8);
			addMicroInstruction(InstructionId::INT_SUB, varnodeRsp, getConstantVarnode(0x8, 0x8), varnodeRsp);
			auto offset = getJumpOffsetToNextInstr();
			addMicroInstruction(InstructionId::STORE, varnodeRsp, getConstantVarnode((uint64_t&)offset, 0x8));
			}*/
			generateInstruction(InstructionId::CALL, varnodeInput0, nullptr);
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

		auto varnodeFlagCond = getConditionFlagVarnode(flagCond);
		auto varnodeNeg = getVirtRegisterVarnode(1);
		generateInstruction(InstructionId::BOOL_NEGATE, varnodeFlagCond, nullptr, varnodeNeg);
		auto varnodeNextInstrOffset = getConstantVarnode(getJumpOffset(0x0), 0x8);
		generateInstruction(InstructionId::CBRANCH, varnodeNextInstrOffset, varnodeNeg);
		auto operand = m_curOperands[1];
		auto varnode = getOperandVarnode(operand, size, nullptr, operand.actions != 0);
		generateGenericOperation(InstructionId::COPY, varnode, nullptr);
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

		auto varnodeFlagCond = getConditionFlagVarnode(flagCond);
		generateGenericOperation(InstructionId::COPY, varnodeFlagCond, nullptr);
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

		auto varnodeFlagCond = getConditionFlagVarnode(flagCond);
		std::shared_ptr<Varnode> varnodeInput0 = getJumpOffsetByOperand(m_curOperands[0]);
		generateInstruction(InstructionId::CBRANCH, varnodeInput0, varnodeFlagCond);
		break;
	}

	default: {
        auto input0 = getConstantVarnode(0x0, 0x8);
		generateInstruction(InstructionId::UNKNOWN, input0);
        std::stringstream message;
        message << "impossible to decode " << m_curInstrView;
        message << " (at 0x" << std::setfill('0') << std::setw(16) << std::right << m_curOffset << ")";
        getCallbacks()->onWarningEmitted(message.str());
	}

	}
}

int ZydisDecoderPcodeX86::getFirstExplicitOperandsCount() const {
    int result = 0;
	for (int i = 0; i < m_curInstr.operand_count; i++) {
		if (m_curOperands[result].visibility != ZYDIS_OPERAND_VISIBILITY_HIDDEN)
			result++;
	}
	return result;
}

std::shared_ptr<Varnode> ZydisDecoderPcodeX86::getJumpOffsetByOperand(const ZydisDecodedOperand& operand) {
    if (operand.type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
		return getOperandVarnode(operand, operand.size / 0x8);
	const auto jmpOffset = static_cast<int>(operand.imm.value.s);
    return getConstantVarnode(getJumpOffset(jmpOffset), 0x8, true);
}

size_t ZydisDecoderPcodeX86::getJumpOffset(int jmpOffset) const {
    const auto offset = m_curOffset + m_curInstr.length + jmpOffset;
	return offset << 8;
}

void ZydisDecoderPcodeX86::generateVectorOperation(const VectorOperationGeneratorInfo& info) {
    const int operationsCount = info.maxSize / info.size;
	auto instrId2 = info.instrId2;
	if (instrId2 == InstructionId::NONE) {
		instrId2 = info.instrId1;
	}

	auto& dstOperand = m_curOperands[0];
	auto& srcOperand = m_curOperands[1];

	for (int i = 0; i < operationsCount; i++) {
		if (info.shuffOp2[i] != -1) {
			int idxOp1 = 0;
			int idxOp2 = 1;
			std::shared_ptr<Varnode> ops[2] = { nullptr, nullptr };

			if (info.isOperationWithSingleOperand) {
				idxOp1 = idxOp2 = (i < operationsCount / 2) ? idxOp1 : idxOp2;
			}
			if (info.instrId1 == InstructionId::COPY || info.instrId1 == InstructionId::FLOAT_SQRT) {
				idxOp1 = idxOp2;
				idxOp2 = -1;
			}

			if (idxOp1 == 0 || idxOp2 == 0) {
				ops[0] = getOperandVarnode(dstOperand, info.size, info.size * info.shuffOp1[i]);
			}
			if (idxOp1 == 1 || idxOp2 == 1) {
				ops[1] = getOperandVarnode(srcOperand, info.size, info.size * info.shuffOp2[i]);
			}

			if (info.isNegate) {
				ops[idxOp1] = generateGenericOperation(InstructionId::INT_NEGATE, ops[idxOp1], nullptr);
			}
			generateGenericOperation(i % 2 ? info.instrId1 : instrId2, ops[idxOp1], idxOp2 != -1 ? ops[idxOp2] : nullptr, nullptr, false, info.size, info.size * i);
		}
    }
}

std::shared_ptr<Varnode> ZydisDecoderPcodeX86::generateGenericOperation(
    InstructionId instrId,
    std::shared_ptr<Varnode> varnodeInput0,
    std::shared_ptr<Varnode> varnodeInput1,
    std::shared_ptr<Varnode> memLocVarnode,
    bool isFictitious,
    int size,
    int offset)
{
    auto& operand = m_curOperands[0];
	if (size == 0x0)
		size = operand.size / 0x8;
	std::shared_ptr<Varnode> varnodeOutput = nullptr;
	if (!isFictitious && operand.type == ZYDIS_OPERAND_TYPE_REGISTER) {
		varnodeOutput = getRegisterVarnode(operand.reg.value, size, offset);
	}
	else {
		varnodeOutput = getVirtRegisterVarnode(size);
	}

	generateInstruction(instrId, varnodeInput0, varnodeInput1, varnodeOutput);

	if (!isFictitious && operand.type == ZYDIS_OPERAND_TYPE_MEMORY) {
        if (!memLocVarnode) {
            memLocVarnode = getOperandVarnode(operand, size, offset, nullptr, false);
        }
        generateInstruction(InstructionId::STORE, memLocVarnode, varnodeOutput);
	}

	return varnodeOutput;
}

Instruction* ZydisDecoderPcodeX86::generateInstruction(
    InstructionId id,
    std::shared_ptr<Varnode> input0,
    std::shared_ptr<Varnode> input1,
    std::shared_ptr<Varnode> output,
    bool zext)
{
    auto instr = Instruction(
		id,
		input0,
		input1,
		output,
		InstructionOffset(m_curOffset, m_curInstrIndex));
    m_curInstrIndex++;

	if (m_curInstrIndex == 1) {
		ZydisFormatter formatter;
		ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
		char buffer[256];
		ZydisFormatterFormatInstruction(
            &formatter,
            &m_curInstr,
            m_curOperands,
            m_curInstr.operand_count,
            buffer,
            sizeof(buffer),
			(ZyanU64)m_curOffset);
		m_curInstrView = buffer;
	}

    m_decodedInstructions.push_back(instr);
	return &m_decodedInstructions.back();
}

std::shared_ptr<Varnode> ZydisDecoderPcodeX86::getOperandVarnode(
    const ZydisDecodedOperand& operand,
    int size,
    std::shared_ptr<Varnode>* memLocVarnode,
    bool isMemLocLoaded,
    int memLocExprSize)
{
    return getOperandVarnode(operand, size, 0x0, memLocVarnode, isMemLocLoaded, memLocExprSize);
}

std::shared_ptr<Varnode> ZydisDecoderPcodeX86::getOperandVarnode(
    const ZydisDecodedOperand& operand,
    int size,
    int offset,
    std::shared_ptr<Varnode>* memLocVarnode,
    bool isMemLocLoaded,
    int memLocExprSize)
{
    if (operand.type == ZYDIS_OPERAND_TYPE_REGISTER) {
		return getRegisterVarnode(operand.reg.value, size, offset);
	}

	if (operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
		return getConstantVarnode(operand.imm.value.u & BitMask(size, 0), size);
	}

	if (operand.type == ZYDIS_OPERAND_TYPE_MEMORY) {
		std::shared_ptr<Varnode> resultVarnode;
		std::shared_ptr<RegisterVarnode> baseRegVarnode = nullptr;
		std::shared_ptr<RegisterVarnode> indexRegVarnode;

		if (operand.mem.base != ZYDIS_REGISTER_NONE) {
			baseRegVarnode = getRegisterVarnode(operand.mem.base, memLocExprSize);
			if (operand.mem.base == ZYDIS_REGISTER_RIP) {
				offset += m_curInstr.length;
			}
		}

		if (operand.mem.index != ZYDIS_REGISTER_NONE) {
			resultVarnode = indexRegVarnode = getRegisterVarnode(operand.mem.index, memLocExprSize);
			if (operand.mem.scale != 1) {
				const auto virtRegVarnode = getVirtRegisterVarnode(memLocExprSize);
                auto input1 = getConstantVarnode(operand.mem.scale & resultVarnode->getMask(), memLocExprSize);
				generateInstruction(InstructionId::INT_MULT, resultVarnode, input1, virtRegVarnode);
				resultVarnode = virtRegVarnode;
			}
			if (baseRegVarnode != nullptr) {
				const auto virtRegVarnode = getVirtRegisterVarnode(memLocExprSize);
				generateInstruction(InstructionId::INT_ADD, baseRegVarnode, resultVarnode, virtRegVarnode);
				resultVarnode = virtRegVarnode;
			}
		}
		else {
			resultVarnode = baseRegVarnode;
		}

		if (operand.mem.disp.has_displacement) {
			const auto constValue = (size_t&)operand.mem.disp.value & resultVarnode->getMask();
			const auto dispVarnode = getConstantVarnode(constValue, memLocExprSize);
			if (resultVarnode != nullptr) {
				const auto virtRegVarnode = getVirtRegisterVarnode(memLocExprSize);
				generateInstruction(InstructionId::INT_ADD, resultVarnode, dispVarnode, virtRegVarnode);
				resultVarnode = virtRegVarnode;
			}
			else {
				resultVarnode = dispVarnode;
			}
		}

		if (offset > 0) {
			const auto virtRegVarnode = getVirtRegisterVarnode(memLocExprSize);
			generateInstruction(InstructionId::INT_ADD, resultVarnode, getConstantVarnode(offset, memLocExprSize), virtRegVarnode);
			resultVarnode = virtRegVarnode;
		}

		if (memLocVarnode) {
			*memLocVarnode = resultVarnode;
		}

		if (isMemLocLoaded) { //check for LEA instruction
			const auto virtRegVarnode = getVirtRegisterVarnode(size);
			generateInstruction(InstructionId::LOAD, resultVarnode, nullptr, virtRegVarnode);
			resultVarnode = virtRegVarnode;
		}
		return resultVarnode;
	}
	return nullptr;
}

std::shared_ptr<Varnode> ZydisDecoderPcodeX86::getConditionFlagVarnode(FlagCond flagCond) {
    std::shared_ptr<Varnode> varnodeCond;

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

		varnodeCond = getRegisterVarnode(flag);
		if (flagCond == FlagCond::NZ || flagCond == FlagCond::NC || flagCond == FlagCond::NP || flagCond == FlagCond::NO) {
			const auto varnodeNeg = getVirtRegisterVarnode(1);
			generateInstruction(InstructionId::BOOL_NEGATE, varnodeCond, nullptr, varnodeNeg);
			varnodeCond = varnodeNeg;
		}
		break;
	}
	case FlagCond::L:
	case FlagCond::LE:
	{
		const auto varnodeNe = getVirtRegisterVarnode(1);
		generateInstruction(InstructionId::INT_NOTEQUAL, getRegisterVarnode(ZYDIS_CPUFLAG_OF), getRegisterVarnode(ZYDIS_CPUFLAG_SF), varnodeNe);
		if (flagCond == FlagCond::LE) {
			const auto varnodeOr = getVirtRegisterVarnode(1);
			generateInstruction(InstructionId::BOOL_OR, getRegisterVarnode(ZYDIS_CPUFLAG_ZF), varnodeNe, varnodeOr);
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
		const auto varnodeEq = getVirtRegisterVarnode(1);
		generateInstruction(InstructionId::INT_EQUAL, getRegisterVarnode(ZYDIS_CPUFLAG_OF), getRegisterVarnode(ZYDIS_CPUFLAG_SF), varnodeEq);
		if (flagCond == FlagCond::NLE) {
			const auto varnodeNeg = getVirtRegisterVarnode(1);
			generateInstruction(InstructionId::BOOL_NEGATE, getRegisterVarnode(ZYDIS_CPUFLAG_ZF), nullptr, varnodeNeg);
			const auto varnodeAnd = getVirtRegisterVarnode(1);
			generateInstruction(InstructionId::BOOL_AND, varnodeEq, varnodeNeg, varnodeAnd);
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
		const auto varnodeOr = getVirtRegisterVarnode(1);
		generateInstruction(InstructionId::BOOL_OR, getRegisterVarnode(ZYDIS_CPUFLAG_CF), getRegisterVarnode(ZYDIS_CPUFLAG_ZF), varnodeOr);
		if (flagCond == FlagCond::A) {
			const auto varnodeNe = getVirtRegisterVarnode(1);
			generateInstruction(InstructionId::BOOL_NEGATE, varnodeOr, nullptr, varnodeNe);
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

std::shared_ptr<RegisterVarnode> ZydisDecoderPcodeX86::getRegisterVarnode(ZydisRegister regId, size_t size, int offset) const {
    auto type = RegisterVarnode::Generic;
    size_t id = 0;
    size_t index = static_cast<size_t>(offset) / 8;
    auto mask = BitMask(size, offset);

	if (regId == ZYDIS_REGISTER_RIP) {
		id = RegisterVarnode::InstructionPointerId;
        type = RegisterVarnode::InstructionPointer;
    }
    else if (regId == ZYDIS_REGISTER_RSP) {
		id = RegisterVarnode::StackPointerId;
        type = RegisterVarnode::StackPointer;
    }
	else if (regId >= ZYDIS_REGISTER_AL && regId <= ZYDIS_REGISTER_BL) {
		id = ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_AL;
	}
	else if (regId >= ZYDIS_REGISTER_AH && regId <= ZYDIS_REGISTER_BH) {
		id = ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_AH;
	}
	else if (regId >= ZYDIS_REGISTER_SPL && regId <= ZYDIS_REGISTER_R15B) {
		id = ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_AH;
	}
	else if (regId >= ZYDIS_REGISTER_AX && regId <= ZYDIS_REGISTER_R15W) {
		id = ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_AX;
	}
	else if (regId >= ZYDIS_REGISTER_EAX && regId <= ZYDIS_REGISTER_R15D) {
		id = ZYDIS_REGISTER_RAX + regId - ZYDIS_REGISTER_EAX;
	}
	else if (regId >= ZYDIS_REGISTER_RAX && regId <= ZYDIS_REGISTER_R15) {
		id = regId;
	}
	else if (regId >= ZYDIS_REGISTER_MM0 && regId <= ZYDIS_REGISTER_MM7) {
		id = regId;
        type = RegisterVarnode::Vector;
	}
	else if (regId >= ZYDIS_REGISTER_XMM0 && regId <= ZYDIS_REGISTER_XMM31) {
		id = regId;
        type = RegisterVarnode::Vector;
	}

    return std::make_shared<RegisterVarnode>(type, id, index, mask, size);
}

std::shared_ptr<RegisterVarnode> ZydisDecoderPcodeX86::getRegisterVarnode(ZydisAccessedFlagsMask flagMask) const {
    return std::make_shared<RegisterVarnode>(
        RegisterVarnode::Flag,
        RegisterVarnode::FlagId,
        0, // index
        BitMask(flagMask),
        1 // register size
    );
}

std::shared_ptr<pcode::RegisterVarnode> ZydisDecoderPcodeX86::getVirtRegisterVarnode(size_t size) const {
    return std::make_shared<RegisterVarnode>(
        RegisterVarnode::Virtual,
        RegisterVarnode::VirtualId,
        0, // index
        BitMask(size, 0),
        size
    );
}

std::shared_ptr<pcode::ConstantVarnode> ZydisDecoderPcodeX86::getConstantVarnode(size_t value, size_t size, bool isAddress) const {
    return std::make_shared<ConstantVarnode>(value, size, isAddress);
}