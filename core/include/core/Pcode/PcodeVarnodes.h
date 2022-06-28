#pragma once
#include <string>
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
        inline const static size_t VirtualId = 100000000;
        inline const static size_t StackPointerId = 100000001;
        inline const static size_t InstructionPointerId = 100000002;
        inline const static size_t FlagId = 100000003;

        enum Type {
            Virtual,
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

        Type getRegType() const;

        size_t getRegId() const;

        size_t getRegIndex() const;

        BitMask getMask() const;

        class Render {
		public:
            virtual std::string getRegisterName(const RegisterVarnode* varnode) const = 0;
        };
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