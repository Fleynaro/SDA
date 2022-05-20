#include "Core/Pcode/PcodeVarnodes.h"

using namespace sda;
using namespace sda::pcode;

Varnode::Varnode(size_t size)
    : m_size(size)
{}

size_t Varnode::getSize() const {
    return m_size;
}

RegisterVarnode::RegisterVarnode(Type type, size_t id, size_t index, BitMask mask, size_t size)
    : Varnode(size), m_type(type), m_id(id), m_index(index), m_mask(mask)
{}

RegisterVarnode::Type RegisterVarnode::getType() const {
    return m_type;
}

size_t RegisterVarnode::getId() const {
    return m_id;
}

size_t RegisterVarnode::getIndex() const {
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