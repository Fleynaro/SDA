#pragma once

namespace sda
{
    class BitMask
    {
    public:
        using Value = size_t;

    private:
        Value m_mask;

    public:
        BitMask(Value mask);

        BitMask(size_t size, size_t offset);

        size_t getMaxSizeInBits() const;

        size_t getOffset() const;

        operator size_t() const;
    };
};