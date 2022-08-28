#include "Core/Pcode/PcodeVarnodes.h"

using namespace sda;
using namespace sda::pcode;

Varnode::Varnode(size_t size)
    : m_size(size)
{}

size_t Varnode::getSize() const {
    return m_size;
}

RegisterVarnode::RegisterVarnode(const Register& reg)
    : Varnode(reg.getSize()), m_register(reg)
{}

bool RegisterVarnode::isRegister() const {
    return true;
}

utils::BitMask RegisterVarnode::getMask() const {
    return m_register.getMask();
}

const Register& RegisterVarnode::getRegister() const {
    return m_register;
}

ConstantVarnode::ConstantVarnode(size_t value, size_t size, bool isAddress)
    : Varnode(size), m_value(value), m_isAddress(isAddress)
{}

bool ConstantVarnode::isRegister() const {
    return false;
}

utils::BitMask ConstantVarnode::getMask() const {
    return utils::BitMask(getSize(), 0);
}

size_t ConstantVarnode::getValue() const {
    return m_value;
}

bool ConstantVarnode::isAddress() const {
    return m_isAddress;
}