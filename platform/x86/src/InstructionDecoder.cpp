#include "SDA/Platform/X86/InstructionDecoder.h"

using namespace sda::platform;

InstructionDecoderX86::InstructionDecoderX86(std::unique_ptr<ZydisDecoder> decoder, std::unique_ptr<ZydisFormatter> formatter)
    : m_decoder(std::move(decoder)), m_formatter(std::move(formatter))
{}

void InstructionDecoderX86::decode(const std::vector<uint8_t>& data) {
    m_decodedInstruction.m_tokens.clear();
    m_decodedInstruction.m_length = 0;

    ZydisDecodedInstruction instr;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];

    auto status = ZydisDecoderDecodeFull(
        m_decoder.get(),
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
        m_formatter.get(),
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