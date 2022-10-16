#pragma once
#include <string>
#include "SDA/Core/Utils/BitMask.h"

namespace sda
{
    class RegisterRepository;

    class Register
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
        utils::BitMask m_mask;
        size_t m_size;

    public:
        Register(Type type, size_t id, size_t index, utils::BitMask mask);

        Type getRegType() const;

        size_t getRegId() const;

        size_t getRegIndex() const;

        utils::BitMask getMask() const;

        size_t getSize() const;

        size_t getBitOffset() const;

        std::string toString(const RegisterRepository* regRepo, bool printSizeAndOffset = true) const;
    };
};