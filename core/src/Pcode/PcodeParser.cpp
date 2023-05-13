#include "SDA/Core/Pcode/PcodeParser.h"
#include "SDA/Core/Platform/RegisterRepository.h"
#include <sstream>
#include <boost/algorithm/string.hpp>

using namespace sda::pcode;
using namespace utils::lexer;

Parser::Parser(utils::lexer::Lexer* lexer, const RegisterRepository* regRepo)
    : AbstractParser(lexer, 3), m_regRepo(regRepo)
{}

std::list<Instruction> Parser::Parse(const std::string& text, const RegisterRepository* regRepo) {
    std::stringstream ss(text);
    IO io(ss, std::cout);
    Lexer lexer(&io);
    Parser parser(&lexer, regRepo);
    parser.init();
    return parser.parse();
}

std::list<Instruction> Parser::parse(char endSymbol) {
    m_curOffset = 0;
    m_instructions.clear();
    m_labelToJump.clear();
    while (!getToken()->isSymbol(endSymbol)) {
        parseLabelIfExists();
        parseInstruction();
        m_curOffset = InstructionOffset(m_curOffset.byteOffset + 1, 0);
    }
    applyLabelJumps();
    // m_instructions to list of instructions
    std::list<Instruction> result;
    for (auto& [offset, instr] : m_instructions) {
        result.push_back(instr);
    }
    return result;
}

void Parser::parseInstruction() {
    std::shared_ptr<Varnode> output;
    std::shared_ptr<Varnode> input0;
    std::shared_ptr<Varnode> input1;

    if (!getToken()->isOneOfKeyword({
        "NOP",
        "STORE",
        "BRANCH",
		"CBRANCH",
		"BRANCHIND",
		"CALL",
		"CALLIND",
		"RETURN",
        "INT"
    })) {
        output = parseRegisterVarnode();
        accept('=');
    }

    auto instrId = parseInstructionId();

    if (instrId != InstructionId::NOP &&
        instrId != InstructionId::RETURN &&
        instrId != InstructionId::INT)
    {
        input0 = parseVarnode();
        if (getToken()->isSymbol(',')) {
            nextToken();
            input1 = parseVarnode();
        }
    }
    
    m_instructions[m_curOffset] = Instruction(instrId, input0, input1, output, m_curOffset);
}

InstructionId Parser::parseInstructionId() {
    std::string name;
    if (getToken()->isIdent(name)) {
        nextToken();
        auto id = magic_enum::enum_cast<InstructionId>(name);
        if (!id.has_value())
            throw error(200, "Invalid instruction id");
        return id.value();
    }
    throw error(201, "Expected instruction id");
}

std::shared_ptr<Varnode> Parser::parseVarnode() {
    if (getToken()->isSymbol('<')) {
        nextToken();
        std::string labelName;
        if (getToken()->isIdent(labelName)) {
            nextToken();
            accept('>');
            auto it = m_labelToJump.find(labelName);
            if (it != m_labelToJump.end()) {
                it->second.startOffsets.push_back(m_curOffset);
            } else {
                m_labelToJump[labelName] = { { m_curOffset }, InvalidOffset };
            }
        }
        return nullptr;
    }
    if (getToken()->type == Token::Const || getToken()->isSymbol('-'))
        return parseConstantVarnode();
    if (getToken()->type == Token::Ident || getToken()->isSymbol('$'))
        return parseRegisterVarnode();
    throw error(300, "Expected varnode");
}

std::shared_ptr<RegisterVarnode> Parser::parseRegisterVarnode() {
    std::string name;
    if (getToken()->isSymbol('$')) {
        nextToken();
        if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
            if (constToken->valueType == ConstToken::Integer) {
                auto regIdx = constToken->value.integer - 1;
                nextToken();
                size_t size = 0;
                size_t offset = 0;
                parseVarnodeSizeOffset(size, offset);
                auto reg = Register(
                    Register::Virtual,
                    Register::VirtualId,
                    regIdx,
                    utils::BitMask(size, offset)
                );
                return std::make_shared<RegisterVarnode>(reg);
            }
        }
    } else if (getToken()->isIdent(name)) {
        nextToken();
        boost::algorithm::to_lower(name);
        try {
            auto regId = m_regRepo->getRegisterId(name);
            auto type = m_regRepo->getRegisterType(regId);
            size_t size = 0;
            size_t offset = 0;
            if (type == Register::Vector) {
                parseVarnodeSizeOffsetOfVector(size, offset);
            } else {
                parseVarnodeSizeOffset(size, offset);
            }
            if (size == 0)
                throw error(401, "Invalid size of register");
            auto reg = Register(
                type,
                regId,
                offset / utils::MaxMaskSizeInBytes,
                utils::BitMask(size, offset)
            );
            return std::make_shared<RegisterVarnode>(reg);
        } catch (const std::out_of_range&) {
            throw error(402, "Invalid register name");
        }
    }
    throw error(403, "Expected register varnode");
}

std::shared_ptr<ConstantVarnode> Parser::parseConstantVarnode() {
    bool isNegative = false;
    if (getToken()->isSymbol('-')) {
        nextToken();
        isNegative = true;
    }
    if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
        size_t value = 0;
        if (constToken->valueType == ConstToken::Integer) {
            value = constToken->value.integer * (isNegative ? -1 : 1);
        } else if (constToken->valueType == ConstToken::Real) {
            reinterpret_cast<double&>(value) = constToken->value.real * (isNegative ? -1 : 1);
        } else {
            throw error(500, "Expected number, got string");
        }
        nextToken();
        size_t size = 0;
        size_t offset = 0;
        parseVarnodeSizeOffset(size, offset);
        value &= utils::BitMask(size, 0);
        auto constVarnode = std::make_shared<ConstantVarnode>(value, size, false);
        return constVarnode;
    }
    throw error(501, "Expected constant varnode");
}

void Parser::parseVarnodeSizeOffset(size_t& size, size_t& offset) {
    if (getToken()->isSymbol(':')) {
        nextToken();
        if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
            if (constToken->valueType == ConstToken::Integer) {
                size = constToken->value.integer;
                nextToken();
                if (getToken()->isSymbol(':')) {
                    nextToken();
                    if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
                        if (constToken->valueType == ConstToken::Integer) {
                            offset = constToken->value.integer;
                            nextToken();
                            return;
                        }
                    }
                    throw error(601, "Expected offset of varnode");
                }
                return;
            }
        }
    }
    throw error(600, "Expected size of varnode");
}

void Parser::parseVarnodeSizeOffsetOfVector(size_t& size, size_t& offset) {
    if (getToken()->isSymbol(':')) {
        nextToken();
        std::string sizeAndOffset;
        if (getToken()->isIdent(sizeAndOffset)) {
            if (sizeAndOffset.size() == 2) {
                if (sizeAndOffset[0] == 'D')
                    size = 4;
                else if (sizeAndOffset[0] == 'Q')
                    size = 8;
                offset = (sizeAndOffset[1] - 'a') * size;
                nextToken();
                return;
            }
        }
    }
    throw error(400, "Expected size & offset of vector register");
}

void Parser::parseLabelIfExists() {
    if (getToken()->isSymbol('<')) {
        nextToken();
        std::string labelName;
        if (getToken()->isIdent(labelName)) {
            nextToken();
            accept('>');
            accept(':');
            auto it = m_labelToJump.find(labelName);
            if (it != m_labelToJump.end()) {
                it->second.endOffset = m_curOffset;
            } else {
                m_labelToJump[labelName] = Jump{ {}, m_curOffset };
            }
        }
    }
}

void Parser::applyLabelJumps() {
    for (auto& [labelName, jump] : m_labelToJump) {
        if (jump.startOffsets.empty())
            throw Exception(700, "Label " + labelName + " is not used");
        if (jump.endOffset == InvalidOffset)
            throw Exception(701, "Label " + labelName + " is not defined");
        for (auto instrOffset : jump.startOffsets) {
            auto& instr = m_instructions[instrOffset];
            if (!instr.isAnyJump()) {
                throw Exception(702, "Label " + labelName + " is not used as jump target");
            }
            auto addressVarnode = std::make_shared<ConstantVarnode>(jump.endOffset, 8, true);
            m_instructions[instrOffset] = Instruction(
                instr.getId(),
                addressVarnode,
                instr.getInput1(),
                instr.getOutput(),
                instrOffset
            );
        }
    }
}