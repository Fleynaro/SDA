#include "Core/Pcode/PcodeParser.h"
#include <boost/algorithm/string.hpp>

using namespace sda::pcode;
using namespace utils::lexer;

Parser::Exception::Exception(const std::string& message)
    : std::exception(message.c_str())
{}

Parser::Parser(IO* io, const PlatformSpec* platformSpec)
    : m_io(io), m_lexer(io), m_platformSpec(platformSpec)
{
    nextToken();
}

std::list<Instruction> Parser::parse() {
    std::list<Instruction> instructions;
    InstructionOffset offset = 0;
    while (!m_token->isEnd()) {
        auto instr = parseInstruction(offset);
        instructions.push_back(instr);
        offset = offset + 1;
    }
    return instructions;
}

Instruction Parser::parseInstruction(InstructionOffset offset) {
    auto output = parseRegisterVarnode();
    accept('=');
    auto instrId = parseInstructionId();
    auto input0 = parseVarnode();
    std::shared_ptr<Varnode> input1;
    if (m_token->isSymbol(',')) {
        nextToken();
        input1 = parseVarnode();
    }
    return Instruction(instrId, input0, input1, output, offset);
}

InstructionId Parser::parseInstructionId() {
    if (auto identToken = dynamic_cast<const IdentToken*>(m_token.get())) {
        auto id = magic_enum::enum_cast<InstructionId>(identToken->name);
        nextToken();
        if (!id.has_value())
            throw error(200, "Invalid instruction id");
        return id.value();
    }
    throw error(201, "Expected instruction id");
}

std::shared_ptr<Varnode> Parser::parseVarnode() {
    if (m_token->type == Token::Const || m_token->isSymbol('-'))
        return parseConstantVarnode();
    if (m_token->type == Token::Ident || m_token->isSymbol('$'))
        return parseRegisterVarnode();
    throw error(300, "Expected varnode");
}

std::shared_ptr<RegisterVarnode> Parser::parseRegisterVarnode() {
    if (m_token->isSymbol('$')) {
        nextToken();
        if (auto regIdentToken = dynamic_cast<const IdentToken*>(m_token.get())) {
            auto regIdxStr = regIdentToken->name.substr(1);
            nextToken();
            auto regIdx = std::stoull(regIdxStr) - 1;
            auto size = parseVarnodeSize();
            return std::make_shared<RegisterVarnode>(
                RegisterVarnode::Virtual,
                RegisterVarnode::VirtualId,
                regIdx,
                BitMask(size, 0),
                size);
        }
    } else if (auto regIdentToken = dynamic_cast<const IdentToken*>(m_token.get())) {
        auto name = regIdentToken->name;
        boost::algorithm::to_lower(name);
        nextToken();
        try {
            auto regId = m_platformSpec->getRegisterId(name);
            auto type = m_platformSpec->getRegisterType(regId);
            size_t size = 0;
            size_t offset = 0;
            if (type == RegisterVarnode::Vector) {
                if (m_token->isSymbol(':')) {
                    nextToken();
                    if (auto sizeAndOffsetToken = dynamic_cast<const IdentToken*>(m_token.get())) {
                        auto sizeAndOffset = sizeAndOffsetToken->name;
                        if (sizeAndOffset.size() == 2) {
                            if (sizeAndOffset[0] == 'D')
                                size = 4;
                            else if (sizeAndOffset[0] == 'Q')
                                size = 8;
                            offset = (sizeAndOffset[1] - 'a') * size;
                            nextToken();
                        }
                    }
                } else {
                    throw error(400, "Expected size of vector register");
                }
            } else {
                size = parseVarnodeSize();
            }
            if (size == 0)
                throw error(401, "Invalid size of register");
            return std::make_shared<RegisterVarnode>(
                type,
                regId,
                offset / MaxMaskSizeInBytes,
                BitMask(size, offset),
                size);
        } catch (const std::out_of_range&) {
            throw error(402, "Invalid register name");
        }
    }
    throw error(403, "Expected register varnode");
}

std::shared_ptr<ConstantVarnode> Parser::parseConstantVarnode() {
    bool isNegative = false;
    if (m_token->isSymbol('-')) {
        nextToken();
        isNegative = true;
    }
    if (auto constToken = dynamic_cast<const ConstToken*>(m_token.get())) {
        size_t value = 0;
        if (constToken->valueType == ConstToken::Integer) {
            value = constToken->value.integer * (isNegative ? -1 : 1);
        } else if (constToken->valueType == ConstToken::Real) {
            reinterpret_cast<double&>(value) = constToken->value.real * (isNegative ? -1 : 1);
        } else {
            throw error(500, "Expected number, got string");
        }
        nextToken();
        auto size = parseVarnodeSize();
        value &= BitMask(size, 0);
        auto constVarnode = std::make_shared<ConstantVarnode>(value, size, false);
        return constVarnode;
    }
    throw error(501, "Expected constant varnode");
}

size_t Parser::parseVarnodeSize() {
    if (m_token->isSymbol(':')) {
        nextToken();
        if (auto constToken = dynamic_cast<const ConstToken*>(m_token.get())) {
            if (constToken->valueType == ConstToken::Integer) {
                auto integer = constToken->value.integer;
                nextToken();
                return integer;
            }
        }
    }
    throw error(600, "Expected size of varnode");
}

void Parser::accept(char symbol) {
    if (!m_token->isSymbol(symbol))
        throw error(700, "Expected symbol '" + std::string(1, symbol) + "'");
    nextToken();
}

Parser::Exception Parser::error(ErrorCode code, const std::string& message) {
    m_io->error(code, message);
    return Exception(message);
}

void Parser::nextToken() {
    m_token = m_lexer.nextToken();
}