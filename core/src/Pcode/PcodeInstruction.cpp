#include "Core/Pcode/PcodeInstruction.h"
#include <sstream>
#include "Core/Utils/IOManip.h"

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

bool Instruction::isComutative() const {
    return
        // integers
        m_id == InstructionId::INT_ADD ||
        m_id == InstructionId::INT_MULT ||
        m_id == InstructionId::INT_XOR ||
        m_id == InstructionId::INT_AND ||
        m_id == InstructionId::INT_OR ||
        m_id == InstructionId::INT_EQUAL ||
        m_id == InstructionId::INT_NOTEQUAL ||
        // floats
        m_id == InstructionId::FLOAT_ADD ||
        m_id == InstructionId::FLOAT_MULT ||
        m_id == InstructionId::FLOAT_EQUAL ||
        m_id == InstructionId::FLOAT_NOTEQUAL;
}

Instruction::Render::Render(const PlatformSpec* platformSpec)
    : m_platformSpec(platformSpec)
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
        if (regVarnode->getRegType() == RegisterVarnode::Virtual) {
            std::stringstream ss;
            ss << "$U" << (regVarnode->getRegIndex() + 1) << ":" << regVarnode->getSize();
            renderToken(ss.str(), Token::VirtRegister);
        } else {
            std::stringstream ss;
            if (regVarnode->getRegType() == pcode::RegisterVarnode::Flag) {
                ss << m_platformSpec->getRegisterFlagName(varnode->getMask()) + ":1";
            } else {
                auto size = regVarnode->getSize();
                auto maskStr = std::to_string(size);
                if (regVarnode->getRegType() == pcode::RegisterVarnode::Vector) {
                    if (size == 4 || size == 8) {
                        maskStr = std::string(size == 4 ? "D" : "Q");
                        maskStr += static_cast<char>('a' + static_cast<char>(regVarnode->getOffset() / (size * BitsInBytes)));
                    }
                }
                if (regVarnode->getRegType() == pcode::RegisterVarnode::StackPointer) {
                    ss << "RSP:" << maskStr;
                } else if (regVarnode->getRegType() == pcode::RegisterVarnode::InstructionPointer) {
                    ss << "RIP:" << maskStr;
                } else {
                    ss << m_platformSpec->getRegisterName(regVarnode->getRegId()) << ":" << maskStr;
                }
            }
            renderToken(ss.str(), Token::Register);
        }
    }
    else if (auto constVarnode = dynamic_cast<const ConstantVarnode*>(varnode)) {
        std::stringstream ss;
        ss << utils::to_hex() << constVarnode->getValue() << ":";
        if (constVarnode->isAddress())
            ss << "addr"; else ss << constVarnode->getSize();
        renderToken(ss.str(), Token::Number);
    }
}

Instruction::StreamRender::StreamRender(std::ostream& output, const PlatformSpec* platformSpec)
    : Render(platformSpec), m_output(output)
{}

void Instruction::StreamRender::renderToken(const std::string& text, Token token) const {
    m_output << text;
}