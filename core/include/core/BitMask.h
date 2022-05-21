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

        operator size_t() const;
    };
};