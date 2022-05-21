#pragma once
#include "Core/BitMask.h"

namespace sda::pcode
{
    class Varnode
    {
        size_t m_size;
    public:
        Varnode(size_t size);

        virtual ~Varnode() {}

        size_t getSize() const;

        BitMask getMask() const;
    };

    class RegisterVarnode : public Varnode
    {
    public:
        inline const static size_t StackPointerId = 100000000;
        inline const static size_t InstructionPointerId = 100000001;
        inline const static size_t FlagId = 100000002;

        enum Type {
			Generic,
			StackPointer,
			InstructionPointer,
			Flag,
			Vector
		};

    private:
        Type m_type;
        size_t m_id;
        size_t m_index;
        BitMask m_mask;

    public:
        RegisterVarnode(Type type, size_t id, size_t index, BitMask mask, size_t size);

        Type getType() const;

        size_t getId() const;

        size_t getIndex() const;

        BitMask getMask() const;
    };

    class SymbolVarnode : public Varnode
    {
    public:
        SymbolVarnode(size_t size);
    };

    class ConstantVarnode : public Varnode
    {
        size_t m_value;
        bool m_isAddress;
    public:
        ConstantVarnode(size_t value, size_t size, bool isAddress);

        size_t getValue() const;

        bool isAddress() const;
    };
};