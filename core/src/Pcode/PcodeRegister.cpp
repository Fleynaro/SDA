#include "Core/Pcode/PcodeRegister.h"

using namespace sda;
using namespace sda::pcode;

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

size_t Register::getOffset() const {
    return m_mask.getOffset() + m_index * 64;
}