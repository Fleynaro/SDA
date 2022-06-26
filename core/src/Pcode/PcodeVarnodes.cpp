#include "Core/Pcode/PcodeVarnodes.h"

using namespace sda;
using namespace sda::pcode;

Varnode::Varnode(size_t size)
    : m_size(size)
{}

size_t Varnode::getSize() const {
    return m_size;
}

BitMask Varnode::getMask() const {
    return BitMask(m_size, 0);
}

RegisterVarnode::RegisterVarnode(Type type, size_t id, size_t index, BitMask mask, size_t size)
    : Varnode(size), m_type(type), m_id(id), m_index(index), m_mask(mask)
{
    if (type == Type::StackPointer) {
        m_id = StackPointerId;
    } else if (type == Type::InstructionPointer) {
        m_id = InstructionPointerId;
    } else if (type == Type::Flag) {
        m_id = FlagId;
    }
}

RegisterVarnode::Type RegisterVarnode::getRegType() const {
    return m_type;
}

size_t RegisterVarnode::getRegId() const {
    return m_id;
}

size_t RegisterVarnode::getRegIndex() const {
    return m_index;
}

BitMask RegisterVarnode::getMask() const {
    return m_mask;
}

SymbolVarnode::SymbolVarnode(size_t size)
    : Varnode(size)
{}

ConstantVarnode::ConstantVarnode(size_t value, size_t size, bool isAddress)
    : Varnode(size), m_value(value), m_isAddress(isAddress)
{}

size_t ConstantVarnode::getValue() const {
    return m_value;
}

bool ConstantVarnode::isAddress() const {
    return m_isAddress;
}