#pragma once
#include "SDA/Core/Platform/Register.h"

namespace sda::pcode
{
    class Varnode
    {
        size_t m_size;
    public:
        Varnode(size_t size);

        virtual bool isRegister() const = 0;

        virtual utils::BitMask getMask() const = 0;

        size_t getSize() const;
    };

    class RegisterVarnode : public Varnode
    {
        Register m_register;
    public:
        RegisterVarnode(const Register& reg);

        bool isRegister() const override;

        utils::BitMask getMask() const override;

        const Register& getRegister() const;
    };

    class ConstantVarnode : public Varnode
    {
        size_t m_value;
        bool m_isAddress;
    public:
        ConstantVarnode(size_t value, size_t size, bool isAddress);

        bool isRegister() const override;

        utils::BitMask getMask() const override;

        size_t getValue() const;

        bool isAddress() const;
    };
};