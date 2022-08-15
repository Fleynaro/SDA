#include "Core/Platform/Register.h"
#include <Core/Platform/RegisterHelper.h>
#include <sstream>

using namespace sda;

Register::Register(Type type, size_t id, size_t index, BitMask mask)
    : m_type(type), m_id(id), m_index(index), m_mask(mask), m_size(mask.getSize())
{
    if (type == Type::StackPointer) {
        m_id = StackPointerId;
    } else if (type == Type::InstructionPointer) {
        m_id = InstructionPointerId;
    } else if (type == Type::Flag) {
        m_id = FlagId;
    }
}

Register::Type Register::getRegType() const {
    return m_type;
}

size_t Register::getRegId() const {
    return m_id;
}

size_t Register::getRegIndex() const {
    return m_index;
}

BitMask Register::getMask() const {
    return m_mask;
}

size_t Register::getSize() const {
    return m_size;
}

size_t Register::getBitOffset() const {
    return m_mask.getOffset() + m_index * 64;
}

std::string Register::toString(const RegisterHelper* regHelper, bool renderSizeAndOffset) const {
    std::stringstream ss;
    if (m_type == Register::Virtual) {
        ss << "$U" << (m_index + 1);
        if (renderSizeAndOffset)
            ss << ":" << m_size;
    } else {
        if (m_type == Register::Flag) {
            ss << regHelper->getRegisterFlagName(m_mask);
        } else {
            if (m_type == Register::StackPointer)
                ss << "RSP";
            else if (m_type == Register::InstructionPointer)
                ss << "RIP";
            else 
                ss << regHelper->getRegisterName(m_id);
        }

        if (renderSizeAndOffset) {
            if (m_type == Register::Vector) {
                if (m_size == 4 || m_size == 8) {
                    ss << (m_size == 4 ? "D" : "Q");
                    ss << static_cast<char>('a' + static_cast<char>(getBitOffset() / (m_size * BitsInBytes)));
                }
            } else {
                ss << ":" << m_size;
            }
        }
    }
    return ss.str();
}