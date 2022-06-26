#include "Disasm/Zydis/ZydisInstructionRenderX86.h"
#include <Zydis/Zydis.h>

using namespace sda::disasm;

ZydisDecoderRenderX86::ZydisDecoderRenderX86(ZydisDecoder* decoder)
    : m_decoder(decoder)
{
	ZydisFormatterInit(&m_formatter, ZYDIS_FORMATTER_STYLE_INTEL);
}

void ZydisDecoderRenderX86::decode(const std::vector<uint8_t>& data) {
    m_decodedInstruction.m_tokens.clear();
    m_decodedInstruction.m_length = 0;

    ZydisDecodedInstruction instr;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];

    auto status = ZydisDecoderDecodeFull(
        m_decoder,
        data.data(),
        data.size(),
        &instr,
        operands,
        ZYDIS_MAX_OPERAND_COUNT_VISIBLE, 
        ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY);

    if (ZYAN_FAILED(status))
        throw std::runtime_error("ZydisDecoderDecodeFull failed");

    char buffer[256];
    const ZydisFormatterToken* token;
    status = ZydisFormatterTokenizeInstruction(
        &m_formatter,
        &instr,
        operands,
        instr.operand_count_visible,
        buffer,
        sizeof(buffer),
        ZYDIS_RUNTIME_ADDRESS_NONE,
        &token);

    if (ZYAN_FAILED(status))
        throw std::runtime_error("ZydisFormatterFormatInstruction failed");

    ZydisTokenType token_type;
    ZyanConstCharPointer token_value = nullptr;
    do
    {
        ZydisFormatterTokenGetValue(token, &token_type, &token_value);
        if (token_type == ZYDIS_TOKEN_MNEMONIC) {
            m_decodedInstruction.m_tokens.push_back({ Instruction::Token::Mneumonic, token_value });
        }
        else if (token_type == ZYDIS_TOKEN_REGISTER) {
            m_decodedInstruction.m_tokens.push_back({ Instruction::Token::Register, token_value });
        }
        else if (token_type == ZYDIS_TOKEN_ADDRESS_ABS) {
            m_decodedInstruction.m_tokens.push_back({ Instruction::Token::AddressAbs, token_value });
        }
        else if (token_type == ZYDIS_TOKEN_ADDRESS_REL) {
            m_decodedInstruction.m_tokens.push_back({ Instruction::Token::AddressRel, token_value });
        }
        else {
            m_decodedInstruction.m_tokens.push_back({ Instruction::Token::Other, token_value });
        }
    } while (ZYAN_SUCCESS(ZydisFormatterTokenNext(&token)));
    
    m_decodedInstruction.m_length = instr.length;
}

std::string GetFlagName(size_t flag) {
    std::string flagName = "flag";
    if ((flag & ZYDIS_CPUFLAG_CF) != 0)
        flagName = "CF";
    else if ((flag & ZYDIS_CPUFLAG_OF) != 0)
        flagName = "OF";
    else if ((flag & ZYDIS_CPUFLAG_SF) != 0)
        flagName = "SF";
    else if ((flag & ZYDIS_CPUFLAG_ZF) != 0)
        flagName = "ZF";
    else if ((flag & ZYDIS_CPUFLAG_AF) != 0)
        flagName = "AF";
    else if ((flag & ZYDIS_CPUFLAG_PF) != 0)
        flagName = "PF";
    return flagName;
}

std::string ZydisRegisterVarnodeRender::getRegisterName(const pcode::RegisterVarnode* varnode) const {
    if (varnode->getRegType() == pcode::RegisterVarnode::Flag)
        return GetFlagName(varnode->getMask()) + ":1";

    auto size = varnode->getSize();
    auto maskStr = std::to_string(size);
    if (varnode->getRegType() == pcode::RegisterVarnode::Vector) {
        if (size == 4 || size == 8) {
            maskStr = std::string(size == 4 ? "D" : "Q");
            auto maskOffset = varnode->getMask().getOffset();
            maskStr += static_cast<char>('a' + static_cast<char>(maskOffset / (size * 8)));
        }
    }

    if (varnode->getRegType() == pcode::RegisterVarnode::StackPointer) {
        return std::string("RSP:") + maskStr;
    } else if (varnode->getRegType() == pcode::RegisterVarnode::InstructionPointer) {
        return std::string("RIP:") + maskStr;
    }

    const auto regId = static_cast<ZydisRegister>(varnode->getRegId());
    return std::string(ZydisRegisterGetString(regId)) + ":" + maskStr;
}