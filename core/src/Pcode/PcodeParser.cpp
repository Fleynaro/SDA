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
    std::list<Instruction> instructions;
    InstructionOffset offset = 0;
    while (!getToken()->isSymbol(endSymbol)) {
        auto instr = parseInstruction(offset);
        instructions.push_back(instr);
        offset = offset + 1;
    }
    return instructions;
}

Instruction Parser::parseInstruction(InstructionOffset offset) {
    std::shared_ptr<Varnode> output;
    std::shared_ptr<Varnode> input0;
    std::shared_ptr<Varnode> input1;

    if (!getToken()->isKeyword("STORE")) {
        output = parseRegisterVarnode();
        accept('=');
    }

    auto instrId = parseInstructionId();

    input0 = parseVarnode();
    if (getToken()->isSymbol(',')) {
        nextToken();
        input1 = parseVarnode();
    }
    
    return Instruction(instrId, input0, input1, output, offset);
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
        if (getToken()->isIdent(name)) {
            nextToken();
            auto regIdxStr = name.substr(1);
            auto regIdx = std::stoull(regIdxStr) - 1;
            auto size = parseVarnodeSize();
            auto reg = Register(
                Register::Virtual,
                Register::VirtualId,
                regIdx,
                utils::BitMask(size, 0)
            );
            return std::make_shared<RegisterVarnode>(reg);
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
        auto size = parseVarnodeSize();
        value &= utils::BitMask(size, 0);
        auto constVarnode = std::make_shared<ConstantVarnode>(value, size, false);
        return constVarnode;
    }
    throw error(501, "Expected constant varnode");
}

size_t Parser::parseVarnodeSize() {
    if (getToken()->isSymbol(':')) {
        nextToken();
        if (auto constToken = dynamic_cast<const ConstToken*>(getToken().get())) {
            if (constToken->valueType == ConstToken::Integer) {
                auto integer = constToken->value.integer;
                nextToken();
                return integer;
            }
        }
    }
    throw error(600, "Expected size of varnode");
}