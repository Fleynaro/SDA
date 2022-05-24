#include "Core/Pcode/PcodeInstruction.h"
#include <sstream>
#include "Core/Utils.h"

using namespace sda::pcode;

InstructionOffset::InstructionOffset(Offset offset)
    : fullOffset(offset)
{}

InstructionOffset::InstructionOffset(Offset byteOffset, size_t index)
    : byteOffset(byteOffset), index(index)
{}

InstructionOffset::operator size_t() const {
    return fullOffset;
}

Instruction::Instruction(
    InstructionId id,
    std::shared_ptr<Varnode> input0,
    std::shared_ptr<Varnode> input1,
    std::shared_ptr<Varnode> output,
    InstructionOffset offset)
    : m_id(id), m_input0(input0), m_input1(input1), m_output(output), m_offset(offset)
{}

InstructionId Instruction::getId() const {
    return m_id;
}

std::shared_ptr<Varnode> Instruction::getInput0() const {
    return m_input0;
}

std::shared_ptr<Varnode> Instruction::getInput1() const {
    return m_input1;
}

std::shared_ptr<Varnode> Instruction::getOutput() const {
    return m_output;
}

InstructionOffset Instruction::getOffset() const {
    return m_offset;
}

bool Instruction::isAnyJump() const {
    return m_id >= InstructionId::BRANCH && m_id <= InstructionId::RETURN;
}

bool Instruction::isBranching() const {
    return m_id >= InstructionId::BRANCH && m_id <= InstructionId::BRANCHIND;
}

Instruction::Render::Render(const RegisterVarnode::Render* registerRender)
    : m_registerRender(registerRender)
{}

void Instruction::Render::render(const Instruction* instruction) const {
    if (instruction->getOutput()) {
        renderVarnode(instruction->getOutput().get());
        renderToken(" = ", Token::Other);
    }

    renderToken(magic_enum::enum_name(instruction->getId()).data(), Token::Mnemonic);

    if (instruction->getInput0()) {
        renderToken(" ", Token::Other);
        renderVarnode(instruction->getInput0().get());
    }
    if (instruction->getInput1()) {
        renderToken(", ", Token::Other);
        renderVarnode(instruction->getInput1().get());
    }
}

void Instruction::Render::renderVarnode(const Varnode* varnode) const {
    if (auto regVarnode = dynamic_cast<const RegisterVarnode*>(varnode)) {
        auto regName = m_registerRender->getRegisterName(regVarnode);
        renderToken(regName, Token::Register);
    }
    else if (auto symbolVarnode = dynamic_cast<const SymbolVarnode*>(varnode)) {
        auto uniqueId = (size_t)symbolVarnode % 10000;
        std::stringstream ss;
        ss << "$U" << uniqueId << ":" << symbolVarnode->getSize();
        renderToken(ss.str(), Token::Variable);
    }
    else if (auto constVarnode = dynamic_cast<const ConstantVarnode*>(varnode)) {
        std::stringstream ss;
        ss << utils::to_hex() << constVarnode->getValue() << ":";
        if (constVarnode->isAddress())
            ss << "addr"; else ss << constVarnode->getSize();
        renderToken(ss.str(), Token::Number);
    }
}

Instruction::StreamRender::StreamRender(std::ostream& output, const RegisterVarnode::Render* registerRender)
    : Render(registerRender), m_output(output)
{}

void Instruction::StreamRender::renderToken(const std::string& text, Token token) const {
    m_output << text;
}